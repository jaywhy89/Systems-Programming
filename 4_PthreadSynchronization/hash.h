
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);

 public:
  void setup(unsigned the_size_log=5);
  void insert(Ele *e);
  Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();
  int get_tablesize();
  int get_hashvalue(Keytype the_key);
  void accumulate(hash h);
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];
}

template<class Ele, class Keytype> 
list<Ele,Keytype> *
hash<Ele,Keytype>::get_list(unsigned the_idx){
  if (the_idx >= my_size){
    fprintf(stderr,"hash<Ele,Keytype>::list() public idx out of range!\n");
    exit (1);
  }
  return &entries[the_idx];
}

template<class Ele, class Keytype> 
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;

  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  return l->lookup(the_key);
}  

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::print(FILE *f){
  unsigned i;

  for (i=0;i<my_size;i++){
    entries[i].print(f);
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::reset(){
  unsigned i;
  for (i=0;i<my_size;i++){
    entries[i].cleanup();
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::cleanup(){
  unsigned i;
  reset();
  delete [] entries;
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::insert(Ele *e){
  entries[HASH_INDEX(e->key(),my_size_mask)].push(e);
}

template<class Ele, class Keytype>
int
hash<Ele,Keytype>::get_hashvalue(Keytype the_key){
  return HASH_INDEX(the_key,my_size_mask);
}

template<class Ele, class Keytype>
int
hash<Ele,Keytype>::get_tablesize(){
  return my_size;
}

template<class Ele, class Keytype>
void
hash<Ele, Keytype>::accumulate(hash h){
  int i;
  for(i = 0; i < h.get_tablesize(); i++){
    int j;
    list<Ele,Keytype>* entry = h.get_list(i);
    int numEle = entry->num_ele();
    for(j = 0; j < numEle ; j++){
      //printf("num of ele %d\n", entry->num_ele());
      Ele* e2 = entry->pop();
      int k = e2->key();
      //printf("k is %d\n", k);
      Ele* e1;
      if (!(e1 = this->lookup(k))){
        this->insert(e2);//if not there insert
        e1 = this->lookup(k);
        //assert(e1);

        //printf("e1 inserted %d\n", e1->count);
      }
      else{
        e1->count += e2->count;
      }
    }
    
  }
}


#endif
