#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <memory>
#include <fstream>
#include <set>
#include "Database.h"

using namespace cwa;
namespace {
std::wstring wide(const std::string& s){if(s.empty())return L"";int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,&w[0],n);w.pop_back();return w;}
std::string utf8(const std::wstring& w){if(w.empty())return "";int n=WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,nullptr,0,nullptr,nullptr);std::string s(n,'\0');WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,&s[0],n,nullptr,nullptr);s.pop_back();return s;}
std::string text(HWND h){int n=GetWindowTextLengthW(h);std::wstring w(n+1,L'\0');GetWindowTextW(h,&w[0],n+1);w.resize(n);return trim(utf8(w));}
void put(HWND h,const std::string& s){SetWindowTextW(h,wide(s).c_str());}
std::string cell(double n){return cwa::fixed(n);}
std::string csv(const std::string& s){std::string out="\"";for(char c:s){if(c=='\"')out+='\"';out+=c;}return out+'\"';}
const COLORREF BG=RGB(15,25,43), FG=RGB(232,238,247), ACCENT=RGB(252,196,66);
enum {NEW_STUDENT=100,OPEN_DB,NEXT,ADD,REMOVE,SAVE,CANCEL,SEARCH,EDIT_STUDENT,EXPORT,DATA_FOLDER};
}

class Application {
    HINSTANCE instance_;
    HWND window_=nullptr;
    HFONT font_=nullptr,titleFont_=nullptr;
    HBRUSH background_=nullptr,white_=nullptr;
    std::unique_ptr<Database> database_;
    std::unique_ptr<Student> draft_;
    std::vector<Student> records_;
    std::vector<HWND> page_;
    std::wstring folder_,dbPath_;
    HWND id_=nullptr,name_=nullptr,code_=nullptr,course_=nullptr,semester_=nullptr,mark_=nullptr,credits_=nullptr;
    HWND list_=nullptr,detail_=nullptr,summary_=nullptr,progress_=nullptr,filter_=nullptr,semesterSummary_=nullptr;
    bool existing_=false,dirty_=false,building_=false;
    int pageNumber_=0;

    HWND control(const wchar_t* klass,const std::string& caption,DWORD style,int x,int y,int w,int h,int id=0,DWORD ex=0) {
        HWND c=CreateWindowExW(ex,klass,wide(caption).c_str(),WS_CHILD|WS_VISIBLE|style,x,y,w,h,window_,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),instance_,nullptr);
        if(!c)throw std::runtime_error("Cannot create a window control.");
        SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(font_),TRUE);page_.push_back(c);return c;
    }
    HWND label(const std::string& s,int x,int y,int w,int h=26){return control(L"STATIC",s,SS_LEFT,x,y,w,h);}
    HWND edit(int x,int y,int w,const std::string& value="",int id=0){HWND h=control(L"EDIT",value,WS_TABSTOP|ES_AUTOHSCROLL,x,y,w,32,id,WS_EX_CLIENTEDGE);SendMessageW(h,EM_SETLIMITTEXT,200,0);return h;}
    HWND button(const std::string& s,int x,int y,int w,int id){return control(L"BUTTON",s,WS_TABSTOP|BS_OWNERDRAW,x,y,w,38,id);}
    HWND table(int x,int y,int w,int h,const std::vector<std::pair<std::string,int>>& columns) {
        HWND t=control(WC_LISTVIEWW,"",WS_TABSTOP|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS,x,y,w,h,0,WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(t,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_DOUBLEBUFFER);
        ListView_SetBkColor(t,RGB(245,248,252));ListView_SetTextBkColor(t,RGB(245,248,252));ListView_SetTextColor(t,RGB(20,35,55));
        int i=0;for(const auto& col:columns){std::wstring wtext=wide(col.first);LVCOLUMNW c={};c.mask=LVCF_TEXT|LVCF_WIDTH;c.pszText=&wtext[0];c.cx=col.second;SendMessageW(t,LVM_INSERTCOLUMNW,i++,reinterpret_cast<LPARAM>(&c));}return t;
    }
    void row(HWND t,int i,const std::vector<std::string>& cells){for(std::size_t j=0;j<cells.size();++j){std::wstring s=wide(cells[j]);LVITEMW item={};item.mask=LVIF_TEXT;item.iItem=i;item.iSubItem=static_cast<int>(j);item.pszText=&s[0];SendMessageW(t,j?LVM_SETITEMW:LVM_INSERTITEMW,0,reinterpret_cast<LPARAM>(&item));}}
    int selected(HWND t){return ListView_GetNextItem(t,-1,LVNI_SELECTED);}
    void clearPage(){building_=true;for(HWND c:page_)DestroyWindow(c);page_.clear();list_=detail_=nullptr;InvalidateRect(window_,nullptr,TRUE);}
    void heading(const std::string& title,const std::string& subtitle) {
        HWND h=label(title,34,26,950,44);SendMessageW(h,WM_SETFONT,reinterpret_cast<WPARAM>(titleFont_),TRUE);
        label(subtitle,36,78,950,26);
        button("+ New student",36,118,178,NEW_STUDENT);button("Saved database",228,118,178,OPEN_DB);button("Open data folder",420,118,178,DATA_FOLDER);
        label("UMaT CWA Studio  |  Independent learning project",36,644,950,24);
    }
    bool discard(){if(!dirty_)return true;return MessageBoxW(window_,L"Discard the unsaved entry? Saved database records will remain unchanged.",L"Unsaved changes",MB_YESNO|MB_ICONQUESTION)==IDYES;}
    void studentPage(){clearPage();pageNumber_=1;draft_.reset();existing_=dirty_=false;heading("Start with the student","Step 1 of 2  /  Enter the student's details, then continue to course results.");
        label("STUDENT ID",58,220,400);id_=edit(58,252,420);
        label("FULL NAME",58,316,400);name_=edit(58,348,650);
        label("Use numerical course marks out of 100. Letter grades are calculated automatically.",58,414,900);
        label("All completed courses = CWA. One semester only = SWA.",58,450,900);
        button("Continue to courses  >",58,522,260,NEXT);building_=false;SetFocus(id_);
    }
    void coursePage(){clearPage();pageNumber_=2;heading("Build the results record",draft_->name()+"  /  "+draft_->id()+"  /  Step 2 of 2");
        label("COURSE CODE",36,182,200);code_=edit(36,212,180);
        label("COURSE TITLE",234,182,500);course_=edit(234,212,480);
        label("SEMESTER / YEAR",36,258,260);semester_=edit(36,288,260,"2026/27 - Semester 1");
        label("MARK / 100",316,258,160);mark_=edit(316,288,160);
        label("CREDIT HOURS",496,258,180);credits_=edit(496,288,160);
        button("Add course +",736,208,234,ADD);button("Remove selected",736,280,234,REMOVE);
        progress_=label("",36,338,940,26);
        list_=table(36,374,934,156,{{"Code",120},{"Course title",260},{"Semester",200},{"Mark",80},{"Credits",80},{"Grade",65},{"Weighted",100}});
        summary_=label("",36,546,940,30);
        button("Save & View Database",36,594,280,SAVE);button("Cancel entry",336,594,170,CANCEL);
        refreshDraft();building_=false;SetFocus(code_);
    }
    void refreshDraft(){ListView_DeleteAllItems(list_);int i=0;for(const auto& c:draft_->courses())row(list_,i++,{c.code(),c.title(),c.semester(),cell(c.mark()),cell(c.credits()),std::string(1,c.grade()),cell(c.weighted())});
        put(progress_,std::to_string(draft_->courses().size())+" course(s) in this entry. Add each course, then save the complete record.");
        put(summary_,"Credits: "+cell(draft_->credits())+"     Weighted marks: "+cell(draft_->weighted())+"     Average of entered courses: "+cell(draft_->average())+"%");
    }
    void databasePage(const std::string& selectID=""){clearPage();pageNumber_=3;draft_.reset();dirty_=false;heading("Your saved database","Student summaries above; course details below. Select a student to inspect their saved results.");
        filter_=edit(36,180,400);button("Search",452,178,120,SEARCH);button("Edit selected",590,178,180,EDIT_STUDENT);button("Export all CSV",788,178,182,EXPORT);
        list_=table(36,232,934,156,{{"Student ID",160},{"Student name",345},{"Courses",100},{"Credits",120},{"Average (%)",170}});
        detail_=table(36,426,934,130,{{"Course code",110},{"Course title",260},{"Semester",190},{"Mark",80},{"Credits",80},{"Grade",65},{"Weighted",110}});
        summary_=label("",36,394,934,26);semesterSummary_=control(L"EDIT","",ES_MULTILINE|ES_READONLY|WS_VSCROLL,36,566,934,65,0,WS_EX_CLIENTEDGE);
        building_=false;refreshRecords(selectID);
    }
    void refreshRecords(const std::string& selectID="") {records_=database_->all(text(filter_));building_=true;ListView_DeleteAllItems(list_);ListView_DeleteAllItems(detail_);int i=0,target=-1;
        for(const auto& s:records_){row(list_,i,{s.id(),s.name(),std::to_string(s.courses().size()),cell(s.credits()),cell(s.average())});if(s.id()==selectID)target=i;++i;}
        put(summary_,std::to_string(records_.size())+" student(s) found. Select a student for course details.");put(semesterSummary_,"");building_=false;
        if(!records_.empty()){if(target<0)target=0;ListView_SetItemState(list_,target,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);showSelected();}
    }
    void showSelected(){int n=selected(list_);if(n<0||n>=static_cast<int>(records_.size()))return;const Student& s=records_[n];ListView_DeleteAllItems(detail_);int i=0;std::set<std::string> semesters;
        for(const auto& c:s.courses()){row(detail_,i++,{c.code(),c.title(),c.semester(),cell(c.mark()),cell(c.credits()),std::string(1,c.grade()),cell(c.weighted())});semesters.insert(c.semester());}
        put(summary_,s.name()+"  |  Credits: "+cell(s.credits())+"  |  Weighted marks: "+cell(s.weighted())+"  |  Average: "+cell(s.average())+"%");
        std::string info="CWA requires ALL completed courses. For one semester only, the average is SWA.\r\n";
        for(const auto& sem:semesters)info+=sem+": SWA "+cell(s.average(sem))+"%  ("+cell(s.credits(sem))+" credits)\r\n";put(semesterSummary_,info);
    }
    void exportCSV(){auto students=database_->all();if(students.empty())throw std::runtime_error("No saved records to export.");wchar_t path[MAX_PATH]=L"umat_results.csv";OPENFILENAMEW dlg={};dlg.lStructSize=sizeof(dlg);dlg.hwndOwner=window_;dlg.lpstrFilter=L"CSV file (*.csv)\0*.csv\0\0";dlg.lpstrFile=path;dlg.nMaxFile=MAX_PATH;dlg.lpstrDefExt=L"csv";dlg.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST;
        if(!GetSaveFileNameW(&dlg))return;
        std::ostringstream out;out<<"\xEF\xBB\xBF"<<"Student ID,Student name,Course code,Course title,Semester,Mark,Credits,Grade,Weighted mark,Student average\r\n";
        // Protect spreadsheet exports from interpreting user-entered text as formulas.
        auto safe=[](std::string s){if(!s.empty()&&(s[0]=='='||s[0]=='+'||s[0]=='-'||s[0]=='@'||s[0]=='\t'))s="'"+s;return csv(s);};
        for(const auto& s:students)for(const auto& c:s.courses())out<<safe(s.id())<<','<<safe(s.name())<<','<<safe(c.code())<<','<<safe(c.title())<<','<<safe(c.semester())<<','<<cell(c.mark())<<','<<cell(c.credits())<<','<<c.grade()<<','<<cell(c.weighted())<<','<<cell(s.average())<<"\r\n";
        std::string bytes=out.str();HANDLE f=CreateFileW(path,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)throw std::runtime_error("Cannot write the selected CSV file.");DWORD wrote=0;BOOL ok=WriteFile(f,bytes.data(),static_cast<DWORD>(bytes.size()),&wrote,nullptr);CloseHandle(f);if(!ok||wrote!=bytes.size())throw std::runtime_error("CSV export did not finish. Check free disk space.");MessageBoxW(window_,L"All saved results were exported successfully.",L"CSV exported",MB_OK|MB_ICONINFORMATION);
    }
    void command(int id,int event){
        if(building_)return;
        if(event==EN_CHANGE&&pageNumber_==1){dirty_=true;return;}
        if(event==EN_CHANGE&&pageNumber_==2){dirty_=true;return;}
        if(event!=BN_CLICKED)return;
        switch(id){
        case NEW_STUDENT:if(discard())studentPage();break;
        case OPEN_DB:if(discard())databasePage();break;
        case DATA_FOLDER:ShellExecuteW(window_,L"open",folder_.c_str(),nullptr,nullptr,SW_SHOWNORMAL);break;
        case NEXT:{std::unique_ptr<Student> s(new Student(text(id_),text(name_)));for(const auto& old:database_->all())if(_stricmp(old.id().c_str(),s->id().c_str())==0)throw std::runtime_error("This ID already exists. Open the database, select the student and choose Edit selected.");draft_=std::move(s);existing_=false;dirty_=true;coursePage();break;}
        case ADD:{Course c(text(code_),text(course_),text(semester_),number(text(mark_)),number(text(credits_)));draft_->add(c);dirty_=true;put(code_,"");put(course_,"");put(mark_,"");put(credits_,"");refreshDraft();SetFocus(code_);break;}
        case REMOVE:{int n=selected(list_);if(n<0)throw std::runtime_error("Select a course in the table first.");draft_->remove(static_cast<std::size_t>(n));dirty_=true;refreshDraft();break;}
        case SAVE:{if(!text(code_).empty()||!text(course_).empty()||!text(mark_).empty()||!text(credits_).empty())throw std::runtime_error("A course is still in the entry fields. Add it, or clear those fields before saving.");database_->save(*draft_,existing_);std::string id=draft_->id();dirty_=false;databasePage(id);break;}
        case CANCEL:if(discard())databasePage();break;
        case SEARCH:refreshRecords();break;
        case EDIT_STUDENT:{int n=selected(list_);if(n<0)throw std::runtime_error("Select a student first.");draft_.reset(new Student(records_[n]));existing_=true;dirty_=false;coursePage();break;}
        case EXPORT:exportCSV();break;
        }
    }
    void drawButton(DRAWITEMSTRUCT* d){COLORREF color=d->CtlID==SAVE||d->CtlID==NEXT||d->CtlID==ADD?ACCENT:RGB(39,60,86);if(d->itemState&ODS_SELECTED)color=RGB(48,140,139);HBRUSH b=CreateSolidBrush(color);FillRect(d->hDC,&d->rcItem,b);DeleteObject(b);SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,color==ACCENT?RGB(15,25,43):FG);SelectObject(d->hDC,font_);wchar_t t[128];GetWindowTextW(d->hwndItem,t,128);RECT r=d->rcItem;DrawTextW(d->hDC,t,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);if(d->itemState&ODS_FOCUS){InflateRect(&r,-4,-4);DrawFocusRect(d->hDC,&r);}}
    LRESULT message(UINT msg,WPARAM wp,LPARAM lp){
        try{switch(msg){
        case WM_COMMAND:command(LOWORD(wp),HIWORD(wp));return 0;
        case WM_NOTIFY:if(!building_&&pageNumber_==3&&reinterpret_cast<NMHDR*>(lp)->hwndFrom==list_&&reinterpret_cast<NMHDR*>(lp)->code==LVN_ITEMCHANGED)showSelected();return 0;
        case WM_DRAWITEM:drawButton(reinterpret_cast<DRAWITEMSTRUCT*>(lp));return TRUE;
        case WM_CTLCOLORSTATIC:{HDC dc=reinterpret_cast<HDC>(wp);if(reinterpret_cast<HWND>(lp)==semesterSummary_){SetTextColor(dc,RGB(20,35,55));SetBkColor(dc,RGB(255,255,255));return reinterpret_cast<LRESULT>(white_);}SetTextColor(dc,FG);SetBkColor(dc,BG);return reinterpret_cast<LRESULT>(background_);}
        case WM_CTLCOLOREDIT:SetTextColor(reinterpret_cast<HDC>(wp),RGB(20,35,55));SetBkColor(reinterpret_cast<HDC>(wp),RGB(255,255,255));return reinterpret_cast<LRESULT>(white_);
        case WM_ERASEBKGND:{RECT r;GetClientRect(window_,&r);FillRect(reinterpret_cast<HDC>(wp),&r,background_);return 1;}
        case WM_CLOSE:if(discard())DestroyWindow(window_);return 0;
        case WM_DESTROY:PostQuitMessage(0);return 0;
        }}catch(const std::exception& e){MessageBoxW(window_,wide(e.what()).c_str(),L"Please check",MB_OK|MB_ICONWARNING);return 0;}
        return DefWindowProcW(window_,msg,wp,lp);
    }
    static LRESULT CALLBACK procedure(HWND h,UINT m,WPARAM w,LPARAM l){Application* app=reinterpret_cast<Application*>(GetWindowLongPtrW(h,GWLP_USERDATA));if(m==WM_NCCREATE){app=static_cast<Application*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);app->window_=h;SetWindowLongPtrW(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(app));}return app?app->message(m,w,l):DefWindowProcW(h,m,w,l);}
public:
    explicit Application(HINSTANCE i):instance_(i){background_=CreateSolidBrush(BG);white_=CreateSolidBrush(RGB(255,255,255));font_=CreateFontW(-17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");titleFont_=CreateFontW(-32,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");}
    ~Application(){database_.reset();DeleteObject(font_);DeleteObject(titleFont_);DeleteObject(background_);DeleteObject(white_);}
    int run(){wchar_t local[MAX_PATH];if(FAILED(SHGetFolderPathW(nullptr,CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE,nullptr,SHGFP_TYPE_CURRENT,local)))throw std::runtime_error("Cannot locate your application data folder.");folder_=std::wstring(local)+L"\\UMaTCWAStudio";if(!CreateDirectoryW(folder_.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("Cannot create the application data folder.");dbPath_=folder_+L"\\umat_cwa.db";database_.reset(new Database(utf8(dbPath_)));
        INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_LISTVIEW_CLASSES};InitCommonControlsEx(&controls);
        WNDCLASSW wc={};wc.lpfnWndProc=procedure;wc.hInstance=instance_;wc.lpszClassName=L"UMaTCWAStudioWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hIcon=LoadIcon(nullptr,IDI_APPLICATION);wc.hbrBackground=background_;if(!RegisterClassW(&wc))throw std::runtime_error("Cannot register the application window.");
        RECT r={0,0,1010,680};DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;AdjustWindowRect(&r,style,FALSE);
        if(!CreateWindowExW(WS_EX_CONTROLPARENT,wc.lpszClassName,L"UMaT CWA Studio",style,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,instance_,this))throw std::runtime_error("Cannot open the application window.");
        databasePage();ShowWindow(window_,SW_SHOW);UpdateWindow(window_);MSG msg={};while(GetMessageW(&msg,nullptr,0,0)>0){if(!IsDialogMessageW(window_,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}return static_cast<int>(msg.wParam);
    }
};
int WINAPI WinMain(HINSTANCE h,HINSTANCE,LPSTR,int){try{Application app(h);return app.run();}catch(const std::exception& e){MessageBoxW(nullptr,wide(e.what()).c_str(),L"UMaT CWA Studio could not start",MB_OK|MB_ICONERROR);return 1;}}
