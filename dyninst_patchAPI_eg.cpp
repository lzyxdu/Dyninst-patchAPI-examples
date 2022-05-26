// DynInst
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_object.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"

#include <stdlib.h>
#include <stdio.h>
#include <iterator>
#include <string>
#include <vector>
#include <iterator>

// patchAPI
#include "PatchMgr.h"

using namespace std;
using namespace Dyninst;
using namespace PatchAPI;


// a simple example, just insert a printf to print rdi's content in hexadecimal
class PrintfSnippet : public Snippet {

public:
  bool generate(Point *pt, Buffer &buffer){
  // assembly code snippet to insert:
  // push   %rbp
  // mov    %rsp,%rbp
  // sub    $0x10,%rsp
  // push   %rdi
  // push   %rsi
  // push   %rdx
  // push   %rcx
  // push   %r8
  // push   %r9
  // push   %rax
  // push   %r10
  // mov    $0x400400,%r10
  // mov    %rdi,%rsi
  // movb   $0x0,0x5(%rsp)    \0
  // movb   $0xa,0x4(%rsp)    \n
  // movb   $0x78,0x3(%rsp)   x
  // movb   $0x25,0x2(%rsp)   %
  // lea    0x2(%rsp),%rdi
  // mov    $0x0,%eax
  // callq  *%r10
  // pop    %r10
  // pop    %rax
  // pop    %r9
  // pop    %r8
  // pop    %rcx
  // pop    %rdx
  // pop    %rsi
  // pop    %rdi
  // leaveq
    uint8_t bytes[100] = 
    "\x55"
    "\x48\x89\xe5"
    "\x48\x83\xec\x10"
    "\x57"
    "\x56"
    "\x52"
    "\x51"
    "\x41\x50"
    "\x41\x51"
    "\x50"
    "\x41\x52"
    "\x49\xc7\xc2\x00\x04\x40\x00"
    "\x48\x89\xfe"
    "\xc6\x44\x24\x05\x00"
    "\xc6\x44\x24\x04\x0a"
    "\xc6\x44\x24\x03\x78"
    "\xc6\x44\x24\x02\x25"
    "\x48\x8d\x7c\x24\x02"
    "\xb8\x00\x00\x00\x00"
    "\x41\xff\xd2"
    "\x41\x5a"
    "\x58"
    "\x41\x59"
    "\x41\x58"
    "\x59"
    "\x5a"
    "\x5e"
    "\x5f"
    "\xc9"
    ;
    for(int i = 0;i<74;i++){
      buffer.push_back(bytes[i]);
    }
    return true;
  }

};



int main(int argc, const char *argv[]) {

  if(argc != 3){
    cerr << "Usage:\n\t" << argv[0] << " <input binary> <output binary>" << endl;
    return 1;
  }

  const char* input_binary = argv[1];
  const char* output_binary = argv[2];

  BPatch bpatch;

  BPatch_binaryEdit* app = bpatch.openBinary(input_binary, false);

  if(app == NULL){
    return 0;
  }

  cout << "app OK" << endl;

  BPatch_image* image = app->getImage();

  if(image == NULL){
    return 0;
  }
  
  cout << "image OK" << endl;

  PatchMgrPtr patchMgr = PatchAPI::convert(image);

  vector<BPatch_object*> objects;

  image->getObjects(objects);

  int ocount = objects.size();

  cout << "objects: " <<  ocount << endl;

  if(ocount <= 0){
    return 0;
  }

  BPatch_object* batchObj = objects[0];

  PatchObject* binobj = PatchAPI::convert(batchObj);

  Patcher patcher(patchMgr);

  PrintfSnippet::Ptr snippet = PrintfSnippet::create(new PrintfSnippet);

  vector<PatchFunction*> functions;

  binobj->funcs(back_inserter(functions));


  for(vector<PatchFunction*>::iterator funIter = functions.begin(); funIter != functions.end(); funIter++){
    PatchFunction *fun = *funIter;
    //insert snippet at main's entry
    if(fun->name() != "main")
    {
        continue;
    }
    vector<Point*> f_entryPoints;
    patchMgr->findPoints(PatchAPI::Scope(fun), PatchAPI::Point::FuncEntry, back_inserter(f_entryPoints));


    cout << fun->name() << " has: " << f_entryPoints.size() << " entry points" << endl;

    for(vector<Point*>::iterator pointIter = f_entryPoints.begin(); pointIter!= f_entryPoints.end(); pointIter++){
      Point* point = *pointIter;
      cerr << "Patching " << fun->name() << endl;
      patcher.add(PushBackCommand::create(point, snippet));
    }

  }

  patcher.commit();

  cout << "Commited" << endl;

  app->writeFile(output_binary);

  cout << "Written" << endl;


}