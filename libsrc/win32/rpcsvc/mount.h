/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _MOUNT_H_RPCGEN
#define _MOUNT_H_RPCGEN

#define RPCGEN_VERSION	199506

#include <rpc/rpc.h>

#define MNTPATHLEN 1024
#define MNTNAMLEN 255
#define FHSIZE 32
#define FHSIZE3 64

typedef char fhandle[FHSIZE];

typedef struct {
	unsigned int fhandle3_len;
	char *fhandle3_val;
} fhandle3;

struct fhstatus {
	u_int fhs_status;
	union {
		fhandle fhs_fhandle;
	} fhstatus_u;
};
typedef struct fhstatus fhstatus;

enum mountstat3 {
	MNT3_OK = 0,
	MNT3ERR_PERM = 1,
	MNT3ERR_NOENT = 2,
	MNT3ERR_IO = 5,
	MNT3ERR_ACCES = 13,
	MNT3ERR_NOTDIR = 20,
	MNT3ERR_INVAL = 22,
	MNT3ERR_NAMETOOLONG = 63,
	MNT3ERR_NOTSUPP = 10004,
	MNT3ERR_SERVERFAULT = 10006
};
typedef enum mountstat3 mountstat3;

struct mountres3_ok {
	fhandle3 fhandle;
	struct {
		unsigned int auth_flavors_len;
		int *auth_flavors_val;
	} auth_flavors;
};
typedef struct mountres3_ok mountres3_ok;

struct mountres3 {
	mountstat3 fhs_status;
	union {
		mountres3_ok mountinfo;
	} mountres3_u;
};
typedef struct mountres3 mountres3;

typedef char *dirpath;

typedef char *name;

typedef struct mountbody *mountlist;

struct mountbody {
	name ml_hostname;
	dirpath ml_directory;
	mountlist ml_next;
};
typedef struct mountbody mountbody;

typedef struct groupnode *groups;

struct groupnode {
	name gr_name;
	groups gr_next;
};
typedef struct groupnode groupnode;

typedef struct exportnode *exports;

struct exportnode {
	dirpath ex_dir;
	groups ex_groups;
	exports ex_next;
};
typedef struct exportnode exportnode;

#ifdef __cplusplus
extern "C" {
#endif
bool_t xdr_fhandle(XDR *, fhandle);
bool_t xdr_fhandle3(XDR *, fhandle3 *);
bool_t xdr_fhstatus(XDR *, fhstatus *);
bool_t xdr_mountstat3(XDR *, mountstat3 *);
bool_t xdr_mountres3_ok(XDR *, mountres3_ok *);
bool_t xdr_mountres3(XDR *, mountres3 *);
bool_t xdr_dirpath(XDR *, dirpath *);
bool_t xdr_name(XDR *, name *);
bool_t xdr_mountlist(XDR *, mountlist *);
bool_t xdr_mountbody(XDR *, mountbody *);
bool_t xdr_groups(XDR *, groups *);
bool_t xdr_groupnode(XDR *, groupnode *);
bool_t xdr_exports(XDR *, exports *);
bool_t xdr_exportnode(XDR *, exportnode *);
#ifdef __cplusplus
};
#endif

#define MOUNTPROG 100005
#define MOUNTVERS 1
#define MOUNTPROC_NULL 0
#define MOUNTPROC_MNT 1
#define MOUNTPROC_DUMP 2
#define MOUNTPROC_UMNT 3
#define MOUNTPROC_UMNTALL 4
#define MOUNTPROC_EXPORT 5
#define MOUNTPROC_EXPORTALL 6
#define MOUNTVERS3 3

#ifdef __cplusplus
extern "C" {
#endif
void *mountproc_null_1(void *, CLIENT *);
void *mountproc_null_1_svc(void *, struct svc_req *);
fhstatus *mountproc_mnt_1(dirpath *, CLIENT *);
fhstatus *mountproc_mnt_1_svc(dirpath *, struct svc_req *);
mountlist *mountproc_dump_1(void *, CLIENT *);
mountlist *mountproc_dump_1_svc(void *, struct svc_req *);
void *mountproc_umnt_1(dirpath *, CLIENT *);
void *mountproc_umnt_1_svc(dirpath *, struct svc_req *);
void *mountproc_umntall_1(void *, CLIENT *);
void *mountproc_umntall_1_svc(void *, struct svc_req *);
exports *mountproc_export_1(void *, CLIENT *);
exports *mountproc_export_1_svc(void *, struct svc_req *);
exports *mountproc_exportall_1(void *, CLIENT *);
exports *mountproc_exportall_1_svc(void *, struct svc_req *);
void *mountproc_null_3(void *, CLIENT *);
void *mountproc_null_3_svc(void *, struct svc_req *);
mountres3 *mountproc_mnt_3(dirpath *, CLIENT *);
mountres3 *mountproc_mnt_3_svc(dirpath *, struct svc_req *);
mountlist *mountproc_dump_3(void *, CLIENT *);
mountlist *mountproc_dump_3_svc(void *, struct svc_req *);
void *mountproc_umnt_3(dirpath *, CLIENT *);
void *mountproc_umnt_3_svc(dirpath *, struct svc_req *);
void *mountproc_umntall_3(void *, CLIENT *);
void *mountproc_umntall_3_svc(void *, struct svc_req *);
exports *mountproc_export_3(void *, CLIENT *);
exports *mountproc_export_3_svc(void *, struct svc_req *);
#ifdef __cplusplus
};
#endif

#endif /* !_MOUNT_H_RPCGEN */
