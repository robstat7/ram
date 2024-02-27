/*
 * 'pc.c' contains the functions for creating a new
 * process.
 */
#include "../include/ram/pc.h"
#include <stdlib.h>

/*
 * struct task_struct *create_process(void);
 *
 * This function creates a new process.
 */
struct process_control_block *create_process(void)
{
	struct process_control_block *task;

	task = (struct process_control_block *) malloc(sizeof(struct process_control_block));
	if (task == NULL) {
        	/* printk("pc: couldn't allocate memory to the task request!\n"); */
	}

	return task;
}

/*
 * void destroy_process(void);
 *
 * This function destroyes a process.
 */
void destroy_process(struct process_control_block *task)
{
	free(task);
}
