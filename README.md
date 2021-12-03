# Переделанная прошивка GyverMOD под OLED дисплей на 128x32px

## исходник: GyverMOD https://github.com/AlexGyver/GyverMOD

### исправлено:
* режим вариватт, не забудьте указать сопротивление                                                                                                                        
* корректный замер напряжения на батарее если работаем через DC-DC повышайку
* оптимизирована прошивка
### добавлено:
* меню
* анимированная индикация заряда батареи
* индикация подключенной зарядки на дисплее
* запрет на парение при зарядке акб
* понижение яркости при ожидании
* кликание зажатием кнопки, теперь не нужно 1000 раз нажимать кнопку
* индикация о разряде аккумулятора 
* игра flappyBird
* меню настроек
* информация об общем времени курения в формате H M S
* время последней затяжки

**ВНИМАНИЕ! ПУТЬ К ПАПКЕ СО СКЕТЧЕМ НЕ ДОЛЖЕН СОДЕРЖАТЬ РУССКИХ СИМВОЛОВ!
ВО ИЗБЕЖАНИЕ ПРОБЛЕМ ПОЛОЖИТЕ ПАПКУ В КОРЕНЬ ДИСКА С.**

**У ПРОШИВКИ ВЕРТИКАЛЬНАЯ ОРИЕНТАЦИЯ ДИСПЛЕЯ**

**если используете только 1 акб, то требуется повышайка, иначе
будут просадки напряжения на мощной нагрузке и контроллер вырубаться.**

### Настройки в прошивке:
* DCDC_Booster 0 - не используем, 1 - используем повышайки
* initial_calibration 0 - выкл, 1 - вкл (калибровка)
* lowBrightTimer  время в секундах за которое диспелей потускнеет прежде чем отключиться 


![vape](https://user-images.githubusercontent.com/42141666/132029994-230cbe05-203b-4c43-b7d8-a6ba3e14a48e.jpg)


* меню:
![menu](https://user-images.githubusercontent.com/42141666/132029898-19e61d16-1524-4856-973d-34bcc036c7fb.jpg)

* вкладка вариватт:
![vw](https://user-images.githubusercontent.com/42141666/132029937-713fb162-0e8b-42d8-9f4e-43b5e49f9129.jpg)
 
* меню настроек:
![settings](https://user-images.githubusercontent.com/42141666/132029962-ce9e564b-bab1-438e-a1ee-cc41e1ee6e0c.jpg)
 
* информация:
![info](https://user-images.githubusercontent.com/42141666/132029978-fa2703da-5cfb-406f-8272-794e4f07dc79.jpg)
 
* игра  flappy bird:
![flappy bird](https://user-images.githubusercontent.com/42141666/132034748-d959209b-2f73-4d7a-9cab-3ad6f5aa4789.jpg)


**программа для конвертации изображений в массив:** 
https://javl.github.io/image2cpp/
**подробнее здесь:**
https://www.youtube.com/watch?v=S8cbIllYc1I&ab_channel=HomeMade-%D0%A2%D0%BE%2C%D1%87%D1%82%D0%BE%D1%82%D1%8B%D0%BC%D0%BE%D0%B6%D0%B5%D1%88%D1%8C%D1%81%D0%B4%D0%B5%D0%BB%D0%B0%D1%82%D1%8C
  * схема для 1 батареи:

![схема для 1 батареи](https://user-images.githubusercontent.com/42141666/116374389-ab88eb00-a827-11eb-9393-03dce8a03cbf.jpg)




  * схема для 2 батарей:

![схема для 2 батареи](https://user-images.githubusercontent.com/42141666/116374416-b0e63580-a827-11eb-8c8c-34a30092c5ac.jpg)

* PCB:

**Bottom layout:**

![bottom_lay](https://user-images.githubusercontent.com/42141666/144609716-8d786d04-b5c4-482b-a3bb-b1aae5afb0d2.jpg)

**Top layout:**

![top_lay](https://user-images.githubusercontent.com/42141666/144609702-9f587aa6-af82-4af2-b94f-7943aae239d6.jpg)


