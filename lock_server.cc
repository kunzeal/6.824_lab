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
  pthread_cond_init(&stat_cond, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}


lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r){
  lock_protocol::status ret=lock_protocol::OK;
  pthread_mutex_lock(&stat_mutex);
  map<lock_protocol::lockid_t, int>::iterator it=lck_stat_map.end();


  if((it=lck_stat_map.find(lid))==lck_stat_map.end())
  {
    //don't exist before, so create first
    lck_stat_map.insert(pair<lock_protocol::lockid_t,int>(lid, 1));
  }
  else
  {
    while(it->second==1)
    {
      pthread_cond_wait(&stat_cond, &stat_mutex);
    }
    it->second=1;
  }

  nacquire++;

  pthread_mutex_unlock(&stat_mutex);

  //set retval
  r=lock_protocol::OK;

  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r) {
  lock_protocol::status ret=lock_protocol::OK;

  pthread_mutex_lock(&stat_mutex);
  map<lock_protocol::lockid_t, int>::iterator it=lck_stat_map.end();

  if((it=lck_stat_map.find(lid))==lck_stat_map.end())//don't exists
  {
    r=lock_protocol::NOENT;
  }
  else if(it->second==0)//already released
  {
    r=lock_protocol::RPCERR;
  }
  else//valid, so release
  {
    nacquire--;
    it->second=0;
    pthread_cond_broadcast(&stat_cond);
  }

  r=lock_protocol::OK;

  pthread_mutex_unlock(&stat_mutex);

  return ret;
}

lock_server::~lock_server() {
  pthread_mutex_destroy(&stat_mutex);
  pthread_cond_destroy(&stat_cond);
}

