#pragma once
#include "Models.h"
#include <memory>
#include <cstring>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dlfcn.h>
#endif
struct sqlite3;
struct sqlite3_stmt;

namespace cwa {
// RAII wrapper: SQLite stays private; callers use Student and Course objects.
class Database {
#ifdef _WIN32
    HMODULE module_=nullptr;
#else
    void* module_=nullptr;
#endif
    sqlite3* db_=nullptr;
    int (*open_)(const char*,sqlite3**)=nullptr;
    int (*close_)(sqlite3*)=nullptr;
    int (*prepare_)(sqlite3*,const char*,int,sqlite3_stmt**,const char**)=nullptr;
    int (*step_)(sqlite3_stmt*)=nullptr;
    int (*finalize_)(sqlite3_stmt*)=nullptr;
    int (*text_)(sqlite3_stmt*,int,const char*,int,void(*)(void*))=nullptr;
    int (*double_)(sqlite3_stmt*,int,double)=nullptr;
    const unsigned char* (*columnText_)(sqlite3_stmt*,int)=nullptr;
    double (*columnDouble_)(sqlite3_stmt*,int)=nullptr;
    const char* (*error_)(sqlite3*)=nullptr;
    template<class T> void symbol(T& target,const char* name) {
#ifdef _WIN32
        // Win32 guarantees GetProcAddress can represent exported function addresses.
        // Copy the representation without an incompatible function-type cast.
        FARPROC address=GetProcAddress(module_,name);
        static_assert(sizeof(target)==sizeof(address),"Unsupported function pointer representation");
        std::memcpy(&target,&address,sizeof(target));
#else
        target=reinterpret_cast<T>(dlsym(module_,name));
#endif
        if(!target)throw std::runtime_error(std::string("SQLite function unavailable: ")+name);
    }
    void check(int rc) const {if(rc!=0)throw std::runtime_error(error_(db_));}
    void release() noexcept {
        if(db_&&close_)close_(db_);
        db_=nullptr;
#ifdef _WIN32
        if(module_)FreeLibrary(module_);
#else
        if(module_)dlclose(module_);
#endif
        module_=nullptr;
    }
    class Statement {
        Database& d_; sqlite3_stmt* s_=nullptr;
    public:
        Statement(Database& d,const std::string& sql):d_(d) {
            int rc=d_.prepare_(d_.db_,sql.c_str(),-1,&s_,nullptr);
            if(rc){if(s_)d_.finalize_(s_);d_.check(rc);}
        }
        ~Statement(){if(s_)d_.finalize_(s_);}
        Statement(const Statement&)=delete;
        Statement& operator=(const Statement&)=delete;
        void bind(int n,const std::string& value) {
            d_.check(d_.text_(s_,n,value.c_str(),-1,reinterpret_cast<void(*)(void*)>(-1)));
        }
        void bind(int n,double value){d_.check(d_.double_(s_,n,value));}
        bool next(){int rc=d_.step_(s_);if(rc==100)return true;if(rc==101)return false;d_.check(rc);return false;}
        std::string text(int col){auto p=d_.columnText_(s_,col);return p?reinterpret_cast<const char*>(p):"";}
        double number(int col){return d_.columnDouble_(s_,col);}
    };
    void exec(const std::string& sql){Statement s(*this,sql);while(s.next()){} }
public:
    explicit Database(const std::string& path) {
        try {
#ifdef _WIN32
            // Load only the Windows system copy, never a DLL beside the executable.
            wchar_t dir[MAX_PATH];UINT n=GetSystemDirectoryW(dir,MAX_PATH);
            if(!n||n>=MAX_PATH)throw std::runtime_error("Cannot locate the Windows system folder.");
            std::wstring dll=std::wstring(dir)+L"\\winsqlite3.dll";
            module_=LoadLibraryW(dll.c_str());
#else
            module_=dlopen("libsqlite3.so.0",RTLD_NOW|RTLD_LOCAL);
#endif
            if(!module_)throw std::runtime_error("SQLite is unavailable. This Windows build requires Windows 10/11 with winsqlite3.dll.");
            symbol(open_,"sqlite3_open");symbol(close_,"sqlite3_close");
            symbol(prepare_,"sqlite3_prepare_v2");symbol(step_,"sqlite3_step");
            symbol(finalize_,"sqlite3_finalize");symbol(text_,"sqlite3_bind_text");
            symbol(double_,"sqlite3_bind_double");symbol(columnText_,"sqlite3_column_text");
            symbol(columnDouble_,"sqlite3_column_double");symbol(error_,"sqlite3_errmsg");
            check(open_(path.c_str(),&db_));
            exec("PRAGMA foreign_keys=ON");exec("PRAGMA busy_timeout=3000");
            exec("CREATE TABLE IF NOT EXISTS students (id TEXT PRIMARY KEY COLLATE NOCASE, name TEXT NOT NULL)");
            exec("CREATE TABLE IF NOT EXISTS courses (student_id TEXT NOT NULL REFERENCES students(id), code TEXT NOT NULL COLLATE NOCASE, title TEXT NOT NULL, semester TEXT NOT NULL, mark REAL NOT NULL CHECK(mark>=0 AND mark<=100), credits REAL NOT NULL CHECK(credits>0 AND credits<=100), PRIMARY KEY(student_id,code))");
        }catch(...){release();throw;}
    }
    ~Database(){release();}
    Database(const Database&)=delete;
    Database& operator=(const Database&)=delete;
    std::vector<Student> all(const std::string& filter="") {
        std::vector<Student> result;
        Statement s(*this,"SELECT id,name FROM students WHERE instr(lower(id),lower(?))>0 OR instr(lower(name),lower(?))>0 ORDER BY name,id");
        s.bind(1,filter);s.bind(2,filter);
        while(s.next()) {
            Student student(s.text(0),s.text(1));
            Statement c(*this,"SELECT code,title,semester,mark,credits FROM courses WHERE student_id=? ORDER BY semester,code");
            c.bind(1,student.id());
            while(c.next())student.add(Course(c.text(0),c.text(1),c.text(2),c.number(3),c.number(4)));
            result.push_back(student);
        }
        return result;
    }
    void save(const Student& student,bool existing) {
        if(student.courses().empty())throw std::runtime_error("Add at least one course before saving.");
        exec("BEGIN IMMEDIATE");
        try {
            if(existing) {
                Statement s(*this,"UPDATE students SET name=? WHERE id=?");s.bind(1,student.name());s.bind(2,student.id());s.next();
                Statement c(*this,"DELETE FROM courses WHERE student_id=?");c.bind(1,student.id());c.next();
            }else {
                Statement s(*this,"INSERT INTO students(id,name) VALUES(?,?)");s.bind(1,student.id());s.bind(2,student.name());s.next();
            }
            for(const auto& c:student.courses()) {
                Statement s(*this,"INSERT INTO courses(student_id,code,title,semester,mark,credits) VALUES(?,?,?,?,?,?)");
                s.bind(1,student.id());s.bind(2,c.code());s.bind(3,c.title());s.bind(4,c.semester());s.bind(5,c.mark());s.bind(6,c.credits());s.next();
            }
            exec("COMMIT");
        }catch(...){try{exec("ROLLBACK");}catch(...){}throw;}
    }
};
}
