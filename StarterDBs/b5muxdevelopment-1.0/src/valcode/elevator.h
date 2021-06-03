
#define MAXELEVS 250

void push_elevator();
void call_elevator();

typedef struct s_Elevator t_Elevator;

struct s_Elevator {

	t_SpaceList *EQueue;
	int update,room;
};

t_Elevator Elevators[MAXELEVS];

static char *rcselevatorh="$Id: elevator.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
