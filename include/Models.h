#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cwa {
inline std::string trim(const std::string& s) {
    auto a=s.find_first_not_of(" \r\n\t");
    return a==std::string::npos ? "" : s.substr(a,s.find_last_not_of(" \r\n\t")-a+1);
}
inline double number(const std::string& text) {
    std::istringstream in(text); double n; char extra;
    if(!(in>>n) || (in>>extra) || !std::isfinite(n))
        throw std::runtime_error("Enter a valid number in each numerical field.");
    return n;
}
inline std::string fixed(double n) {
    std::ostringstream out; out<<std::fixed<<std::setprecision(2)<<n; return out.str();
}
class Course {
    std::string code_, title_, semester_;
    double mark_, credits_;
public:
    Course(std::string code, std::string title, std::string semester, double mark, double credits)
        : code_(trim(code)), title_(trim(title)), semester_(trim(semester)), mark_(mark), credits_(credits) {
        if(code_.empty() || title_.empty() || semester_.empty())
            throw std::runtime_error("Enter the course code, title and semester.");
        if(!std::isfinite(mark_) || mark_<0 || mark_>100)
            throw std::runtime_error("The course mark must be between 0 and 100.");
        if(!std::isfinite(credits_) || credits_<=0 || credits_>100)
            throw std::runtime_error("Credits must be greater than 0 and at most 100.");
        std::transform(code_.begin(),code_.end(),code_.begin(),[](unsigned char c){return static_cast<char>(std::toupper(c));});
    }
    const std::string& code() const {return code_;}
    const std::string& title() const {return title_;}
    const std::string& semester() const {return semester_;}
    double mark() const {return mark_;}
    double credits() const {return credits_;}
    double weighted() const {return mark_*credits_;}
    char grade() const {return mark_>=80?'A':mark_>=70?'B':mark_>=60?'C':mark_>=50?'D':'F';}
};
class Student {
    std::string id_, name_;
    std::vector<Course> courses_;
public:
    Student(std::string id, std::string name):id_(trim(id)),name_(trim(name)) {
        if(id_.empty() || name_.empty()) throw std::runtime_error("Enter the student ID and full name.");
    }
    const std::string& id() const {return id_;}
    const std::string& name() const {return name_;}
    const std::vector<Course>& courses() const {return courses_;}
    void add(const Course& c) {
        // One result per course. Repeated/replacement attempts require separate policy handling.
        for(const auto& old:courses_) if(old.code()==c.code())
            throw std::runtime_error("This course code is already entered. Remove the old entry first.");
        courses_.push_back(c);
    }
    void remove(std::size_t i) {if(i>=courses_.size())throw std::runtime_error("Select a course first.");courses_.erase(courses_.begin()+i);}
    double credits(const std::string& semester="") const {
        double t=0;for(const auto& c:courses_)if(semester.empty()||semester==c.semester())t+=c.credits();return t;
    }
    double weighted(const std::string& semester="") const {
        double t=0;for(const auto& c:courses_)if(semester.empty()||semester==c.semester())t+=c.weighted();return t;
    }
    double average(const std::string& semester="") const {
        double t=credits(semester);return t?weighted(semester)/t:0;
    }
};
}
