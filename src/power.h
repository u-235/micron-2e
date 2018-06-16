/**
 * \file
 * \brief Интерфейс контроля питания.
 * \details
 * \date Создан 24.02.2018
 * \author Nick Egorrov
 * \copyright GNU Public License 3
 */

#ifndef SRC_POWER_H_
#define SRC_POWER_H_

#ifdef _cplusplus
extern "C" {
#endif

/**
 * Инициализация модуля.
 */
extern void PowerInit();

/**
 * Обновление внутреннего состояния модуля.
 * \param event Набор флагов CLOCK_EVENT_xx см. clock.h
 */
extern void PowerClockEvent(unsigned char event);

/**
 * Проверка состояния источника питания.
 * \return 0 если уровень питания в норме.
 */
extern char PowerCheck();

/**
 *
 * \return Battery voltage in hundredth of volts.
 */
extern unsigned int PowerVoltage();

/**
 *
 * \return Battery charge in percent.
 */
extern unsigned char PowerCharge();

#ifdef _cplusplus
}
#endif

#endif /* SRC_POWER_H_ */
