/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _RQUOTA_H_RPCGEN
#define _RQUOTA_H_RPCGEN

#define RPCGEN_VERSION	199506

#include <rpc/rpc.h>

#define RQ_PATHLEN 1024

struct getquota_args {
	char *gqa_pathp;
	int gqa_uid;
};
typedef struct getquota_args getquota_args;
#define RQUOTA_MAXQUOTAS 0x02
#define RQUOTA_USRQUOTA 0x00
#define RQUOTA_GRPQUOTA 0x01

struct ext_getquota_args {
	char *gqa_pathp;
	int gqa_type;
	int gqa_id;
};
typedef struct ext_getquota_args ext_getquota_args;

struct rquota {
	int rq_bsize;
	bool_t rq_active;
	u_int rq_bhardlimit;
	u_int rq_bsoftlimit;
	u_int rq_curblocks;
	u_int rq_fhardlimit;
	u_int rq_fsoftlimit;
	u_int rq_curfiles;
	u_int rq_btimeleft;
	u_int rq_ftimeleft;
};
typedef struct rquota rquota;

enum gqr_status {
	Q_OK = 1,
	Q_NOQUOTA = 2,
	Q_EPERM = 3
};
typedef enum gqr_status gqr_status;

struct getquota_rslt {
	gqr_status status;
	union {
		rquota gqr_rquota;
	} getquota_rslt_u;
};
typedef struct getquota_rslt getquota_rslt;

#ifdef __cplusplus
extern "C" {
#endif
bool_t xdr_getquota_args(XDR *, getquota_args *);
bool_t xdr_ext_getquota_args(XDR *, ext_getquota_args *);
bool_t xdr_rquota(XDR *, rquota *);
bool_t xdr_gqr_status(XDR *, gqr_status *);
bool_t xdr_getquota_rslt(XDR *, getquota_rslt *);
#ifdef __cplusplus
};
#endif

#define RQUOTAPROG 100011
#define RQUOTAVERS 1
#define RQUOTAPROC_GETQUOTA 1
#define RQUOTAPROC_GETACTIVEQUOTA 2
#define EXT_RQUOTAVERS 2

#ifdef __cplusplus
extern "C" {
#endif
getquota_rslt *rquotaproc_getquota_1(getquota_args *, CLIENT *);
getquota_rslt *rquotaproc_getquota_1_svc(getquota_args *, struct svc_req *);
getquota_rslt *rquotaproc_getactivequota_1(getquota_args *, CLIENT *);
getquota_rslt *rquotaproc_getactivequota_1_svc(getquota_args *, struct svc_req *);
getquota_rslt *rquotaproc_getquota_2(ext_getquota_args *, CLIENT *);
getquota_rslt *rquotaproc_getquota_2_svc(ext_getquota_args *, struct svc_req *);
getquota_rslt *rquotaproc_getactivequota_2(ext_getquota_args *, CLIENT *);
getquota_rslt *rquotaproc_getactivequota_2_svc(ext_getquota_args *, struct svc_req *);
#ifdef __cplusplus
};
#endif

#endif /* !_RQUOTA_H_RPCGEN */
