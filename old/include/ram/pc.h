struct process_control_block {
	int state;
	long counter;
	int priority;
};

struct process_control_block *create_process(void);
void destroy_process(struct process_control_block *task);
