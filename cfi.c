#include "FreeRTOS.h"
#include "task.h"
#include "cfi.h"

extern int stop_rev;

int printf(const char *fmt, ...);

#if( configCFI_CHECK == 1 )

void prvErrorHandle(CFIState_t *pxCFI)
{
	if (pxCFI->xErrHandleMode == SUSPEND_TASK) {
		printf("CFI Error! Suspend current task...\n");
		vTaskSuspend(NULL);
	}
	if (pxCFI->xErrHandleMode == PAUSE_REV) {
		stop_rev = 5;
	}
}

/* cfi.c 函数调用执行前后执行 */
void __cyg_profile_func_enter(void *this_func, void *call_site)
{
	CFIState_t *pxCFI = xTaskGetCFIState(NULL);

	if (!pxCFI->pxShadowStack) return;
	if (pxCFI->usJobCount) return;

	*((pxCFI->pxShadowStack)++) = (uint32_t)call_site;
}

/* cfi.c 函数调用返回前执行 */
void __cyg_profile_func_exit(void *this_func, void *call_site)
{
	CFIState_t *pxCFI = xTaskGetCFIState(NULL);
	uint32_t tmp;

	if (!pxCFI->pxShadowStack) return;
	if (pxCFI->usJobCount) return;

	tmp = *(--(pxCFI->pxShadowStack));

	if (tmp != (uint32_t)call_site)
		prvErrorHandle(pxCFI);
}

void vCFIJobCountIncrement(void)
{
	CFIState_t *pxCFI = xTaskGetCFIState(NULL);
	pxCFI->usJobCount = (pxCFI->usJobCount+1) % pxCFI->usCheckFrequency;
}

/* CFI初始化函数 */
void vCFIInit(TaskHandle_t xTask,
			  uint16_t usFrequency,
			  ErrHandleMode_t xMode,
			  ErrHandler_t xFunction)
{
	StackType_t *pxStack;
	// 利用FreeRTOS提供的内存管理API初始化CFI检查栈
	pxStack = pvPortMalloc( ((size_t)configMINIMAL_STACK_SIZE) * sizeof(StackType_t) );

	if (pxStack) {
		// 获取Task中CFIState的地址
		CFIState_t *pxCFI = xTaskGetCFIState(xTask);
		// 进行CFIState初始化工作
		pxCFI->pxShadowStack = pxStack;
		pxCFI->usCheckFrequency = usFrequency;
		pxCFI->usJobCount = 0;
		pxCFI->xErrHandleMode = xMode;
		pxCFI->xErrHandler = xFunction;
	} else
		vPortFree(pxStack);
}

#endif
