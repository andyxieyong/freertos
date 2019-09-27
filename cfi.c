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

/* cfi.c ��������ִ��ǰ��ִ�� */
void __cyg_profile_func_enter(void *this_func, void *call_site)
{
	CFIState_t *pxCFI = xTaskGetCFIState(NULL);

	if (!pxCFI->pxShadowStack) return;
	if (pxCFI->usJobCount) return;

	*((pxCFI->pxShadowStack)++) = (uint32_t)call_site;
}

/* cfi.c �������÷���ǰִ�� */
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

/* CFI��ʼ������ */
void vCFIInit(TaskHandle_t xTask,
			  uint16_t usFrequency,
			  ErrHandleMode_t xMode,
			  ErrHandler_t xFunction)
{
	StackType_t *pxStack;
	// ����FreeRTOS�ṩ���ڴ����API��ʼ��CFI���ջ
	pxStack = pvPortMalloc( ((size_t)configMINIMAL_STACK_SIZE) * sizeof(StackType_t) );

	if (pxStack) {
		// ��ȡTask��CFIState�ĵ�ַ
		CFIState_t *pxCFI = xTaskGetCFIState(xTask);
		// ����CFIState��ʼ������
		pxCFI->pxShadowStack = pxStack;
		pxCFI->usCheckFrequency = usFrequency;
		pxCFI->usJobCount = 0;
		pxCFI->xErrHandleMode = xMode;
		pxCFI->xErrHandler = xFunction;
	} else
		vPortFree(pxStack);
}

#endif
