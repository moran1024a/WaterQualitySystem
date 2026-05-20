#include "auth.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

/* minimal MD5 */
typedef struct { uint32_t a,b,c,d; uint64_t bits; unsigned char buf[64]; } MD5Ctx;
static uint32_t F(uint32_t x,uint32_t y,uint32_t z){return (x&y)|(~x&z);} static uint32_t G(uint32_t x,uint32_t y,uint32_t z){return (x&z)|(y&~z);} static uint32_t H(uint32_t x,uint32_t y,uint32_t z){return x^y^z;} static uint32_t I(uint32_t x,uint32_t y,uint32_t z){return y^(x|~z);} static uint32_t R(uint32_t x,int s){return (x<<s)|(x>>(32-s));}
static void md5_block(MD5Ctx*c,const unsigned char b[64]){uint32_t a=c->a,bv=c->b,cv=c->c,d=c->d,x[16];int i;static const uint32_t t[]={0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,0x698098d8,0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,0xa679438e,0x49b40821,0xf61e2562,0xc040b340,0x265e5a51,0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,0xfcefa3f8,0x676f02d9,0x8d2a4c8a,0xfffa3942,0x8771f681,0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,0xbebfbc70,0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,0xf4292244,0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391};
for(i=0;i<16;i++)x[i]=(uint32_t)b[i*4]|((uint32_t)b[i*4+1]<<8)|((uint32_t)b[i*4+2]<<16)|((uint32_t)b[i*4+3]<<24);
#define OP(f,a,b,c,d,xk,s,ti) a=b+R(a+f(b,c,d)+xk+ti,s)
OP(F,a,bv,cv,d,x[0],7,t[0]);OP(F,d,a,bv,cv,x[1],12,t[1]);OP(F,cv,d,a,bv,x[2],17,t[2]);OP(F,bv,cv,d,a,x[3],22,t[3]);OP(F,a,bv,cv,d,x[4],7,t[4]);OP(F,d,a,bv,cv,x[5],12,t[5]);OP(F,cv,d,a,bv,x[6],17,t[6]);OP(F,bv,cv,d,a,x[7],22,t[7]);OP(F,a,bv,cv,d,x[8],7,t[8]);OP(F,d,a,bv,cv,x[9],12,t[9]);OP(F,cv,d,a,bv,x[10],17,t[10]);OP(F,bv,cv,d,a,x[11],22,t[11]);OP(F,a,bv,cv,d,x[12],7,t[12]);OP(F,d,a,bv,cv,x[13],12,t[13]);OP(F,cv,d,a,bv,x[14],17,t[14]);OP(F,bv,cv,d,a,x[15],22,t[15]);
OP(G,a,bv,cv,d,x[1],5,t[16]);OP(G,d,a,bv,cv,x[6],9,t[17]);OP(G,cv,d,a,bv,x[11],14,t[18]);OP(G,bv,cv,d,a,x[0],20,t[19]);OP(G,a,bv,cv,d,x[5],5,t[20]);OP(G,d,a,bv,cv,x[10],9,t[21]);OP(G,cv,d,a,bv,x[15],14,t[22]);OP(G,bv,cv,d,a,x[4],20,t[23]);OP(G,a,bv,cv,d,x[9],5,t[24]);OP(G,d,a,bv,cv,x[14],9,t[25]);OP(G,cv,d,a,bv,x[3],14,t[26]);OP(G,bv,cv,d,a,x[8],20,t[27]);OP(G,a,bv,cv,d,x[13],5,t[28]);OP(G,d,a,bv,cv,x[2],9,t[29]);OP(G,cv,d,a,bv,x[7],14,t[30]);OP(G,bv,cv,d,a,x[12],20,t[31]);
OP(H,a,bv,cv,d,x[5],4,t[32]);OP(H,d,a,bv,cv,x[8],11,t[33]);OP(H,cv,d,a,bv,x[11],16,t[34]);OP(H,bv,cv,d,a,x[14],23,t[35]);OP(H,a,bv,cv,d,x[1],4,t[36]);OP(H,d,a,bv,cv,x[4],11,t[37]);OP(H,cv,d,a,bv,x[7],16,t[38]);OP(H,bv,cv,d,a,x[10],23,t[39]);OP(H,a,bv,cv,d,x[13],4,t[40]);OP(H,d,a,bv,cv,x[0],11,t[41]);OP(H,cv,d,a,bv,x[3],16,t[42]);OP(H,bv,cv,d,a,x[6],23,t[43]);OP(H,a,bv,cv,d,x[9],4,t[44]);OP(H,d,a,bv,cv,x[12],11,t[45]);OP(H,cv,d,a,bv,x[15],16,t[46]);OP(H,bv,cv,d,a,x[2],23,t[47]);
OP(I,a,bv,cv,d,x[0],6,t[48]);OP(I,d,a,bv,cv,x[7],10,t[49]);OP(I,cv,d,a,bv,x[14],15,t[50]);OP(I,bv,cv,d,a,x[5],21,t[51]);OP(I,a,bv,cv,d,x[12],6,t[52]);OP(I,d,a,bv,cv,x[3],10,t[53]);OP(I,cv,d,a,bv,x[10],15,t[54]);OP(I,bv,cv,d,a,x[1],21,t[55]);OP(I,a,bv,cv,d,x[8],6,t[56]);OP(I,d,a,bv,cv,x[15],10,t[57]);OP(I,cv,d,a,bv,x[6],15,t[58]);OP(I,bv,cv,d,a,x[13],21,t[59]);OP(I,a,bv,cv,d,x[4],6,t[60]);OP(I,d,a,bv,cv,x[11],10,t[61]);OP(I,cv,d,a,bv,x[2],15,t[62]);OP(I,bv,cv,d,a,x[9],21,t[63]);
#undef OP
c->a+=a;c->b+=bv;c->c+=cv;c->d+=d;}
static void md5_init(MD5Ctx*c){c->a=0x67452301;c->b=0xefcdab89;c->c=0x98badcfe;c->d=0x10325476;c->bits=0;}
static void md5_update(MD5Ctx*c,const unsigned char*in,size_t n){size_t i=(size_t)((c->bits>>3)&63U),fill=64U-i;c->bits+=((uint64_t)n<<3); if(n>=fill){memcpy(c->buf+i,in,fill);md5_block(c,c->buf);in+=fill;n-=fill;while(n>=64U){md5_block(c,in);in+=64U;n-=64U;}i=0U;} memcpy(c->buf+i,in,n);} 
static void md5_final(MD5Ctx*c,unsigned char out[16]){unsigned char pad[64]={0x80},len[8];size_t i;for(i=0;i<8;i++)len[i]=(unsigned char)((c->bits>>(8*i))&0xff); md5_update(c,pad,1+((119-((size_t)(c->bits>>3)%64))%64)); md5_update(c,len,8); for(i=0;i<4;i++){out[i]=(unsigned char)(c->a>>(8*i));out[4+i]=(unsigned char)(c->b>>(8*i));out[8+i]=(unsigned char)(c->c>>(8*i));out[12+i]=(unsigned char)(c->d>>(8*i));}}

static int wq_ensure_parent_user_dir(void){return wq_ensure_directory("data")==WQ_SUCCESS&&wq_ensure_directory("data/auth")==WQ_SUCCESS?WQ_SUCCESS:WQ_ERROR;}

/* keep existing store funcs unchanged from previous where valid */
int wq_user_store_init(UserStore *store, size_t initial_capacity){ if(!store) return WQ_ERROR; store->users=NULL;store->count=0;store->capacity=0; if(initial_capacity>0) return wq_user_store_reserve(store,initial_capacity); return WQ_SUCCESS; }
void wq_user_store_destroy(UserStore *store){ if(!store) return; if(store->users){wq_secure_clear(store->users,store->capacity*sizeof(UserAccount)); free(store->users);} store->users=NULL;store->count=0;store->capacity=0; }
void wq_user_store_clear(UserStore *store){ if(!store) return; store->count=0; }
int wq_user_store_reserve(UserStore *store, size_t new_capacity){ UserAccount *p; if(!store) return WQ_ERROR; if(new_capacity<=store->capacity) return WQ_SUCCESS; if(new_capacity>WQ_MAX_USERS) return WQ_ERROR; p=(UserAccount*)realloc(store->users,new_capacity*sizeof(UserAccount)); if(!p) return WQ_ERROR; store->users=p; store->capacity=new_capacity; return WQ_SUCCESS; }
int wq_user_store_push(UserStore *store, const UserAccount *account){ size_t nc; if(!store||!account) return WQ_ERROR; if(store->count>=store->capacity){ nc=store->capacity?store->capacity*2U:WQ_DEFAULT_USER_COUNT; if(nc>WQ_MAX_USERS) nc=WQ_MAX_USERS; if(nc<=store->capacity||wq_user_store_reserve(store,nc)!=WQ_SUCCESS) return WQ_ERROR;} store->users[store->count++]=*account; return WQ_SUCCESS; }
UserAccount *wq_find_user(UserStore *store, const char *username){ size_t i; if(!store||!username) return NULL; for(i=0;i<store->count;i++) if(strcmp(store->users[i].username,username)==0) return &store->users[i]; return NULL; }
const UserAccount *wq_find_user_const(const UserStore *store, const char *username){ size_t i; if(!store||!username) return NULL; for(i=0;i<store->count;i++) if(strcmp(store->users[i].username,username)==0) return &store->users[i]; return NULL; }

int wq_generate_salt(unsigned char salt[WQ_SALT_LENGTH]){ size_t i; if(!salt) return WQ_ERROR; for(i=0;i<WQ_SALT_LENGTH;i++) salt[i]=(unsigned char)(rand()&0xFF); return WQ_SUCCESS; }
int wq_md5_with_salt(const char *password,const unsigned char salt[WQ_SALT_LENGTH],char hash_out[WQ_MAX_PASSWORD_HASH_LENGTH]){ MD5Ctx c; unsigned char d[16]; size_t i; if(!password||!salt||!hash_out) return WQ_ERROR; md5_init(&c); md5_update(&c,(const unsigned char*)password,strlen(password)); md5_update(&c,salt,WQ_SALT_LENGTH); md5_final(&c,d); for(i=0;i<16;i++) sprintf(hash_out+i*2,"%02x",d[i]); hash_out[32]='\0'; return WQ_SUCCESS; }
bool wq_secure_hash_equal(const char *a,const char *b){ size_t i,n; unsigned char x=0; if(!a||!b) return false; n=strlen(a); if(n!=strlen(b)) return false; for(i=0;i<n;i++) x|=(unsigned char)(a[i]^b[i]); return x==0; }
void wq_secure_clear(void *buffer, size_t size){ volatile unsigned char *p=(volatile unsigned char*)buffer; size_t i; if(!buffer) return; for(i=0;i<size;i++) p[i]=0; }

int wq_add_builtin_users(UserStore *store){ UserAccount a,g; memset(&a,0,sizeof(a)); memset(&g,0,sizeof(g)); if(!store) return WQ_ERROR; strncpy(a.username,"admin",sizeof(a.username)-1); a.role=WQ_ROLE_ADMIN; a.source=WQ_USER_SOURCE_BUILTIN; a.status=WQ_USER_STATUS_ACTIVE; strncpy(g.username,"guest",sizeof(g.username)-1); g.role=WQ_ROLE_GUEST; g.source=WQ_USER_SOURCE_BUILTIN; g.status=WQ_USER_STATUS_ACTIVE; return (wq_user_store_push(store,&a)==WQ_SUCCESS&&wq_user_store_push(store,&g)==WQ_SUCCESS)?WQ_SUCCESS:WQ_ERROR; }

int wq_load_user_store(UserStore *store, const char *filename){ FILE *fp; UserFileHeader h; uint32_t i; if(!store||!filename) return WQ_ERROR; fp=fopen(filename,"rb"); if(!fp) return WQ_SUCCESS; if(fread(&h,sizeof(h),1,fp)!=1){fclose(fp);return WQ_ERROR;} if(memcmp(h.magic,WQ_USER_FILE_MAGIC,8)!=0||h.version!=WQ_USER_FILE_VERSION){fclose(fp);return WQ_ERROR;} for(i=0;i<h.count;i++){ UserFileRecord r; UserAccount a; if(fread(&r,sizeof(r),1,fp)!=1){fclose(fp);return WQ_ERROR;} memset(&a,0,sizeof(a)); strncpy(a.username,r.username,sizeof(a.username)-1); strncpy(a.password_hash,r.password_hash,sizeof(a.password_hash)-1); memcpy(a.salt,r.salt,WQ_SALT_LENGTH); a.role=(WQUserRole)r.role; a.status=(WQUserStatus)r.status; a.source=WQ_USER_SOURCE_CREATED; if(wq_user_store_push(store,&a)!=WQ_SUCCESS){fclose(fp);return WQ_ERROR;} }
 fclose(fp); return WQ_SUCCESS; }
int wq_save_created_users(const UserStore *store, const char *filename){ FILE *fp; UserFileHeader h; uint32_t i,cnt=0,idx=0; if(!store||!filename) return WQ_ERROR; if(wq_ensure_parent_user_dir()!=WQ_SUCCESS) return WQ_ERROR; for(i=0;i<store->count;i++) if(store->users[i].source==WQ_USER_SOURCE_CREATED) cnt++; fp=fopen(filename,"wb"); if(!fp) return WQ_ERROR; memset(&h,0,sizeof(h)); memcpy(h.magic,WQ_USER_FILE_MAGIC,8); h.version=WQ_USER_FILE_VERSION; h.count=cnt; if(fwrite(&h,sizeof(h),1,fp)!=1){fclose(fp);return WQ_ERROR;} for(i=0;i<store->count;i++) if(store->users[i].source==WQ_USER_SOURCE_CREATED){ UserFileRecord r; memset(&r,0,sizeof(r)); strncpy(r.username,store->users[i].username,sizeof(r.username)-1); strncpy(r.password_hash,store->users[i].password_hash,sizeof(r.password_hash)-1); memcpy(r.salt,store->users[i].salt,WQ_SALT_LENGTH); r.role=(uint32_t)store->users[i].role; r.status=(uint32_t)store->users[i].status; if(fwrite(&r,sizeof(r),1,fp)!=1){fclose(fp);return WQ_ERROR;} idx++; }
 if(idx!=cnt){fclose(fp);return WQ_ERROR;} return fclose(fp)==0?WQ_SUCCESS:WQ_ERROR; }
int wq_auth_startup(UserStore *store){ if(!store) return WQ_ERROR; srand((unsigned int)time(NULL)); if(wq_user_store_init(store,WQ_DEFAULT_USER_COUNT)!=WQ_SUCCESS) return WQ_ERROR; if(wq_add_builtin_users(store)!=WQ_SUCCESS){wq_user_store_destroy(store);return WQ_ERROR;} if(wq_load_user_store(store,WQ_USER_BIN_FILE)!=WQ_SUCCESS){wq_user_store_destroy(store);return WQ_ERROR;} return WQ_SUCCESS; }
int wq_auth_shutdown(UserStore *store){ int r; if(!store) return WQ_ERROR; r=wq_save_created_users(store,WQ_USER_BIN_FILE); wq_user_store_destroy(store); return r; }

bool wq_verify_login(const UserStore *store,const char *username,const char *password,UserAccount *out){ const UserAccount *a; char h[WQ_MAX_PASSWORD_HASH_LENGTH]; if(!store||!username||!password||!out) return false; a=wq_find_user_const(store,username); if(!a||a->status!=WQ_USER_STATUS_ACTIVE) return false; if(a->source==WQ_USER_SOURCE_BUILTIN){ if((strcmp(username,"admin")==0&&strcmp(password,"123456")==0)||(strcmp(username,"guest")==0&&strcmp(password,"guest")==0)){*out=*a;return true;} return false; } if(wq_md5_with_salt(password,a->salt,h)!=WQ_SUCCESS) return false; if(wq_secure_hash_equal(h,a->password_hash)){*out=*a; return true;} return false; }
int wq_login(const UserStore *store, UserAccount *out){ char u[WQ_MAX_USERNAME_LENGTH],p[WQ_MAX_PASSWORD_LENGTH]; size_t t; if(!store||!out) return WQ_ERROR; for(t=0;t<WQ_MAX_LOGIN_ATTEMPTS;t++){ printf("用户名: "); if(!fgets(u,sizeof(u),stdin)) return WQ_ERROR; printf("密码: "); if(!fgets(p,sizeof(p),stdin)) return WQ_ERROR; wq_trim(u); wq_trim(p); if(wq_verify_login(store,u,p,out)){ wq_secure_clear(p,sizeof(p)); return WQ_SUCCESS;} printf("登录失败，还可尝试%lu次\n",(unsigned long)(WQ_MAX_LOGIN_ATTEMPTS-t-1)); } wq_secure_clear(p,sizeof(p)); return WQ_ERROR; }
int wq_create_user(UserStore *store,const char *username,const char *password,WQUserRole role){ UserAccount a; if(!store||!username||!password) return WQ_ERROR; if(username[0]=='\0'||password[0]=='\0') return WQ_ERROR; if(strlen(username)>=WQ_MAX_USERNAME_LENGTH||strlen(password)>=WQ_MAX_PASSWORD_LENGTH) return WQ_ERROR; if(role!=WQ_ROLE_ADMIN&&role!=WQ_ROLE_GUEST&&role!=WQ_ROLE_USER) return WQ_ERROR; if(wq_find_user(store,username)) return WQ_ERROR; memset(&a,0,sizeof(a)); strncpy(a.username,username,sizeof(a.username)-1); a.role=role; a.source=WQ_USER_SOURCE_CREATED; a.status=WQ_USER_STATUS_ACTIVE; if(wq_generate_salt(a.salt)!=WQ_SUCCESS) return WQ_ERROR; if(wq_md5_with_salt(password,a.salt,a.password_hash)!=WQ_SUCCESS) return WQ_ERROR; if(wq_user_store_push(store,&a)!=WQ_SUCCESS) return WQ_ERROR; return wq_save_created_users(store,WQ_USER_BIN_FILE); }
int wq_export_users_to_csv(const UserStore *store,const char *csv_filename,bool include_builtin_users){ FILE *fp; size_t i,j; if(!store||!csv_filename) return WQ_ERROR; if(wq_ensure_directory("output")!=WQ_SUCCESS) return WQ_ERROR; fp=fopen(csv_filename,"w"); if(!fp) return WQ_ERROR; fprintf(fp,"username,role,source,status,salt_hex,password_md5\n"); for(i=0;i<store->count;i++){ const UserAccount *a=&store->users[i]; if(!include_builtin_users&&a->source==WQ_USER_SOURCE_BUILTIN) continue; fprintf(fp,"%s,%u,%u,%u,",a->username,(unsigned)a->role,(unsigned)a->source,(unsigned)a->status); for(j=0;j<WQ_SALT_LENGTH;j++) fprintf(fp,"%02x",a->salt[j]); fprintf(fp,",%s\n",a->password_hash); } return fclose(fp)==0?WQ_SUCCESS:WQ_ERROR; }

/* permissions and strings reuse prior logic */
bool wq_role_can_execute(WQUserRole role, WQOperation operation)
{
    if (operation == WQ_OP_NONE) return false;
    if (operation == WQ_OP_LOGIN || operation == WQ_OP_SYSTEM_STARTUP ||
        operation == WQ_OP_EXIT_SYSTEM || operation == WQ_OP_CLEAR_SCREEN) return true;
    if (role == WQ_ROLE_ADMIN) return true;
    if (role == WQ_ROLE_GUEST) {
        return operation == WQ_OP_VIEW_OVERVIEW || operation == WQ_OP_VIEW_ANALYSIS_REPORT;
    }
    if (role == WQ_ROLE_USER) {
        return operation != WQ_OP_USER_MENU &&
               operation != WQ_OP_CREATE_USER &&
               operation != WQ_OP_EXPORT_USERS &&
               operation != WQ_OP_RESTORE_DATA &&
               operation != WQ_OP_MODIFY_DATA &&
               operation != WQ_OP_DELETE_DATA;
    }
    return false;
}
bool wq_user_can_execute(const UserAccount *user, WQOperation operation){ return user&&user->status==WQ_USER_STATUS_ACTIVE&&wq_role_can_execute(user->role,operation);} int wq_require_permission(const UserAccount *user, WQOperation operation){ return wq_user_can_execute(user,operation)?WQ_SUCCESS:WQ_ERROR; }
WQOperation wq_main_menu_option_to_operation(int menu_option){ switch(menu_option){case 1:return WQ_OP_DATA_MENU;case 2:return WQ_OP_PREPROCESS_MENU;case 3:return WQ_OP_STATISTICS_MENU;case 4:return WQ_OP_PREDICTION_MENU;case 5:return WQ_OP_VIEW_OVERVIEW;case 6:return WQ_OP_VIEW_WARNING_REPORT;case 7:return WQ_OP_VIEW_ANALYSIS_REPORT;case 8:return WQ_OP_BACKUP_MENU;case 9:return WQ_OP_CLEAR_SCREEN;case 10:return WQ_OP_USER_MENU;case 11:return WQ_OP_EXPORT_USERS;case 0:return WQ_OP_EXIT_SYSTEM;default:return WQ_OP_NONE;}}
const char *wq_role_to_string(WQUserRole role){ return role==WQ_ROLE_ADMIN?"管理员":(role==WQ_ROLE_USER?"普通用户":(role==WQ_ROLE_GUEST?"访客":"未知角色")); }
const char *wq_operation_to_string(WQOperation op){
    switch(op){
    case WQ_OP_DATA_MENU:return "数据基础操作菜单";
    case WQ_OP_PREPROCESS_MENU:return "数据预处理菜单";
    case WQ_OP_STATISTICS_MENU:return "统计分析菜单";
    case WQ_OP_PREDICTION_MENU:return "预测分析菜单";
    case WQ_OP_BACKUP_MENU:return "备份恢复菜单";
    case WQ_OP_USER_MENU:return "用户管理菜单";
    case WQ_OP_PREPROCESS_DATA:return "数据预处理";
    case WQ_OP_MODIFY_DATA:return "修改数据";
    case WQ_OP_DELETE_DATA:return "删除数据";
    case WQ_OP_SORT_DATA:return "排序数据";
    case WQ_OP_SAVE_DATA:return "保存数据";
    case WQ_OP_BACKUP_DATA:return "数据备份";
    case WQ_OP_RESTORE_DATA:return "数据恢复";
    case WQ_OP_CREATE_USER:return "创建用户";
    case WQ_OP_EXPORT_USERS:return "导出用户";
    case WQ_OP_VIEW_OVERVIEW:return "查看数据概览";
    case WQ_OP_VIEW_WARNING_REPORT:return "查看预警报告";
    case WQ_OP_VIEW_ANALYSIS_REPORT:return "查看分析报告";
    default:return "操作";
    }
}
