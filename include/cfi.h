#ifndef FREERTOS_INCLUDE_CFI_H_
#define FREERTOS_INCLUDE_CFI_H_

typedef TaskFunction_t	ErrHandler_t;

typedef enum {
	PAUSE_REV,
	SUSPEND_TASK
} ErrHandleMode_t;

typedef struct {
	StackType_t			*pxShadowStack;
	uint16_t			usCheckFrequency;
	uint16_t			usJobCount;
	ErrHandleMode_t 	xErrHandleMode;
	ErrHandler_t		xErrHandler;
} CFIState_t;

void vCFIJobCountIncrement(void);

void vCFIInit(TaskHandle_t xTask,
			  uint16_t usFrequency,
			  ErrHandleMode_t xMode,
			  ErrHandler_t xFunction);

#endif /* FREERTOS_INCLUDE_CFI_H_ */
