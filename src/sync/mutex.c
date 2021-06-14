#include "mutex.h"
#include "semaphore.h"

void mutex_init(struct mutex *mut) {
	sema_init(&mut->sema, 1);
}

void mutex_down(struct mutex *mut) {
	sema_down(&mut->sema);
}

bool mutex_try_down(struct mutex *mut) {
	return sema_try_down(&mut->sema);
}

void mutex_up(struct mutex *mut) {
	if(mut->sema.value != 0) {
		return;
	} else {
		sema_up(&mut->sema);
	}
}