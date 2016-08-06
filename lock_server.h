// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <map>
#include <iterator>
#include <string>
#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"

typedef struct {
    int status;
    pthread_cond_t cond;
}lock_stat;

class lock_server {

 protected:
  int nacquire;

 public:
  lock_server();
  ~lock_server();
  lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int &);
  lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int &);

 private:
  std::map<lock_protocol::lockid_t, lock_stat*> lck_stat_map;
  pthread_mutex_t stat_mutex;
  lock_stat *create_lock_stat(lock_protocol::lockid_t lid);
  int acquire(lock_stat *lst);
};

#endif 







