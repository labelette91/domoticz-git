struct sched_param {
	int __sched_priority ;
}sched_param;

int  sched_setscheduler( int,int, struct sched_param* );
int sched_get_priority_max(int);
#define SCHED_RR 0

#define SCHED_OTHER 0
