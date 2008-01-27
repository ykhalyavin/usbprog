#ifndef __BSR_H__
#define __BSR_H__

#define BSR_IO_HI			1
#define BSR_IO_LO			0


/* cell type identification */
enum cell_type {
	BSR_CELL_BC0 = 0,
	BSR_CELL_BC1,
	BSR_CELL_BC2,
	BSR_CELL_BC3,
	BSR_CELL_BC4,
	BSR_CELL_BC5,
	BSR_CELL_BC6,
	BSR_CELL_BC7,
	BSR_CELL_BC8,
	BSR_CELL_BC9,
	BSR_CELL_BC10,
};

/* primary function of a cell */
enum cell_func {
	BSR_FUNC_INPUT = 0,
	BSR_FUNC_CLOCK,
	BSR_FUNC_OUTPUT2,
	BSR_FUNC_OUTPUT3,
	BSR_FUNC_CONTROL,
	BSR_FUNC_CONTROLR,
	BSR_FUNC_INTERNAL,
	BSR_FUNC_BIDIR,
	BSR_FUNC_OBSERVE_ONLY,
}

/* disable value of a port */
enum cell_disable_value {
	BSR_DIS_RESULT_Z = 0,	// high-impedance state
	BSR_DIS_RESULT_WEAK0,	// weak ¡°0¡± external pull down
	BSR_DIS_RESULT_WEAK1,	// weak ¡°1¡± external pull up
	BSR_DIS_RESULT_PULL0,	// weak ¡°0¡± internal pull down
	BSR_DIS_RESULT_PULL1,	// weak ¡°1¡± internal pull up 
	BSR_DIS_RESULT_KEEPER,	// ¡°kept¡± state memory of the last strongly driven logic state
};

typedef struct bsr_struct_ {
	char cell;			// select from enum cell_type
	int  port;			// -1 indicate a controller
	char func;			// select from enum cell_func
	char safe;			// -1 indicate that don't care about the value
	int  controller;	// if port is not a controller, indicate the controller's cell
	char disable;		// disable value, select from enum cell_disable_value
	char result;		// indicate the port's state when relevant controller's value set as disable 
} str_bsr;

#endif  /* __BSR_H__ */