// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

lock_server::lock_server():
  nacquire (0)
{
  lck_stat_map.clear();
  pthread_mutex_init(&stat_mutex, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_stat *
lock_server::create_lock_stat(lock_protocol::lockid_t lid)
{
  //init lock_stat as val
  lock_stat *lst=(lock_stat*)malloc(sizeof(lock_stat));
  lst->status=0;
  pthread_cond_init(&lst->cond, NULL);

  //init pair
  pair<lock_protocol::lockid_t, lock_stat*> p;
  p.first=lid;
  p.second=lst;

  //insert to lck_stat_map;
  lck_stat_map.insert(p);

  return lst;
}

int
lock_server::acquire(lock_stat *lst) {
  if(lst->status==0)
  {
    lst->status=1;
    nacquire++;
    return 0;
  }
  else
  {
    while(lst->status==1)
    {
      pthread_cond_wait(&lst->cond, &stat_mutex);
    }
    lst->status=1;
    nacquire++;
  }

  return 0;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r){
  lock_protocol::status ret=lock_protocol::OK;
  pthread_mutex_lock(&stat_mutex);
  map<lock_protocol::lockid_t, lock_stat*>::iterator it=lck_stat_map.end();

  lock_stat *lst=NULL;

  if((it=lck_stat_map.find(lid))==lck_stat_map.end())
    //don't exist before, so create first
    lst=create_lock_stat(lid);
  else
    lst=it->second;

  acquire(lst);

  pthread_mutex_unlock(&stat_mutex);

  //set retval
  r=lock_protocol::OK;

  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r) {
  lock_protocol::status ret=lock_protocol::OK;
  r=lock_protocol::OK;
  pthread_mutex_lock(&stat_mutex);
  map<lock_protocol::lockid_t, lock_stat *>::iterator it=lck_stat_map.end();

  if((it=lck_stat_map.find(lid))==lck_stat_map.end())//don't exists
  {
    r=lock_protocol::NOENT;
  }
  else if(it->second->status==0)//already released
  {
    r=lock_protocol::RPCERR;
  }
  else//valid, so release
  {
    printf("relase %lld\n", lid);
    nacquire--;
    it->second->status=0;
    pthread_cond_signal(&it->second->cond);
  }

  pthread_mutex_unlock(&stat_mutex);

  return ret;
}

lock_server::~lock_server() {
  printf("deconstruct\n");
  /* destroy all status and its condition variables */
  map<lock_protocol::lockid_t, lock_stat*>::iterator it;
  for(it=lck_stat_map.end();it!=lck_stat_map.end();it++)
  {
    pthread_cond_destroy(&it->second->cond);
    free(it->second);
  }

  pthread_mutex_destroy(&stat_mutex);
}

