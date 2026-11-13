#include "../include/Database.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <functional>
using namespace cwa;
bool rejects(const std::function<void()>& f){try{f();return false;}catch(const std::exception&){return true;}}
int main(){
 const char* path="cwa_test_temporary.db";std::remove(path);
 Student s("EEE001","Joseph Tetteh");
 s.add(Course("EL 201","Circuit Theory","S1",80,3));
 s.add(Course("MA 201","Engineering Mathematics","S1",70,3));
 s.add(Course("EP 201","Engineering Practice","S1",90,2));
 assert(s.credits()==8&&s.weighted()==630&&s.average()==78.75);
 assert(rejects([&]{s.add(Course("el 201","Duplicate","S2",75,3));}));
 assert(rejects([]{Course c("X","X","S",101,3);}));
 assert(rejects([]{Course c("X","X","S",70,0);}));
 assert(rejects([]{Course c("X","X","S",70,-1);}));
 assert(rejects([]{Student s(" ","Name");}));
 assert(rejects([]{number("75abc");}));
 assert(rejects([]{number("nan");}));
 assert(number(" 75.5 ")==75.5);
 for(int i=0;i<=100;++i){Course c("X","X","S",i,1);assert(c.grade()==(i>=80?'A':i>=70?'B':i>=60?'C':i>=50?'D':'F'));}
 {
  Database d(path);d.save(s,false);assert(d.all().size()==1);
  assert(d.all("joseph").size()==1&&d.all("EEE").size()==1&&d.all("nobody").empty());
  assert(rejects([&]{d.save(s,false);})); // Duplicate insert rolls back atomically.
  assert(d.all()[0].courses().size()==3);
  s.add(Course("EL 202","Machines","S2",60,4));d.save(s,true);
  assert(d.all()[0].average()==72.5&&d.all()[0].average("S1")==78.75);
  assert(d.all()[0].average("S2")==60);
  Student special("ID'2","O'Neil; DROP TABLE students;");
  special.add(Course("CODE","Title \"quoted\"","S1",0,0.5));d.save(special,false);
  assert(d.all().size()==2&&d.all("O'Neil").size()==1);
  Student lower("eee001","Duplicate");lower.add(Course("X","X","S1",90,3));
  assert(rejects([&]{d.save(lower,false);}));assert(d.all().size()==2);
  Student empty("EMPTY","Empty");assert(rejects([&]{d.save(empty,false);}));
 }
 std::cout<<"Model, grade boundaries, input, weighted average, SQLite CRUD and rollback checks passed.\n";
 {
  Database reopened(path);auto found=reopened.all("EEE001");assert(found.size()==1);assert(found[0].average()==72.5);
  Student changed=found[0];changed.remove(0);reopened.save(changed,true);
  assert(reopened.all("EEE001")[0].courses().size()==3);
 }
 std::remove(path);std::cout<<"Restart persistence and course removal checks passed.\n";
}
