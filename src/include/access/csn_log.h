/*
 * csn_log.h
 *
 * Commit-Sequence-Number log.
 *
 * Portions Copyright (c) 1996-2014, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/csn_log.h
 */
#ifndef CSNLOG_H
#define CSNLOG_H

#include "access/xlog.h"
#include "utils/snapshot.h"

/* XLOG stuff */
#define XLOG_CSN_ASSIGNMENT		0x00
#define XLOG_CSN_SETXIDCSN		0x10
#define XLOG_CSN_ZEROPAGE		0x20
#define XLOG_CSN_TRUNCATE		0x30

/*
 * We should log MAX generated CSN to wal, so that database will not generate
 * a historical CSN after database restart. This may appear when system time
 * turned back.
 *
 * However we can not log the MAX CSN every time it generated, if so it will
 * cause too many wal expend, so we log it 5s more in the future.
 *
 * As a trade off, when this database restart, there will be 5s bad performance
 * for time synchronization among sharding nodes.
 *
 * It looks like we can redefine this as a configure parameter, and the user
 * can decide which way they prefer.
 *
 */
#define	CSN_ASSIGN_TIME_INTERVAL	5

typedef struct xl_xidcsn_set
{
	XidCSN		  xidcsn;
	TransactionId xtop;			/* XID's top-level XID */
	int			nsubxacts;		/* number of subtransaction XIDs */
	TransactionId xsub[FLEXIBLE_ARRAY_MEMBER];	/* assigned subxids */
} xl_xidcsn_set;

#define MinSizeOfXidCSNSet offsetof(xl_xidcsn_set, xsub)
#define	CSNAddByNanosec(csn,second) (csn + second * 1000000000L)

extern void CSNLogSetCSN(TransactionId xid, int nsubxids,
							   TransactionId *subxids, XidCSN csn, bool write_xlog);
extern XidCSN CSNLogGetCSNByXid(TransactionId xid);

extern Size CSNLogShmemSize(void);
extern void CSNLogShmemInit(void);
extern void BootStrapCSNLog(void);
extern void ShutdownCSNLog(void);
extern void CheckPointCSNLog(void);
extern void ExtendCSNLog(TransactionId newestXact);
extern void TruncateCSNLog(TransactionId oldestXact);

extern void csnlog_redo(XLogReaderState *record);
extern void csnlog_desc(StringInfo buf, XLogReaderState *record);
extern const char *csnlog_identify(uint8 info);
extern void WriteAssignCSNXlogRec(XidCSN xidcsn);
extern void set_last_max_csn(XidCSN xidcsn);
extern void set_last_log_wal_csn(XidCSN xidcsn);
extern XidCSN get_last_log_wal_csn(void);
extern void prepare_csn_env(bool enable, bool same, TransactionId *xmin_for_csn_in_control);
extern void CatchCSNLog(void);
extern void ActivateCSNlog(void);
extern void DeactivateCSNlog(void);
extern void StartupCSN(void);
extern void CompleteCSNInitialization(void);
extern void CSNlogParameterChange(bool newvalue, bool oldvalue);
extern bool get_csnlog_status(void);

#endif   /* CSNLOG_H */