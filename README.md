# переделанная прошивка GyverMOD под OLED дисплей на 128x32 px

## исходник: GyverMOD https://github.com/AlexGyver/GyverMOD

**ВНИМАНИЕ! ПУТЬ К ПАПКЕ СО СКЕТЧЕМ НЕ ДОЛЖЕН СОДЕРЖАТЬ РУССКИХ СИМВОЛОВ!
ВО ИЗБЕЖАНИЕ ПРОБЛЕМ ПОЛОЖИТЕ ПАПКУ В КОРЕНЬ ДИСКА С.**

**если используете только 1 акб, требуется повышайка, без нее напряжение
будет проседать на мощной нагрузке и мк вырубаться.**

### Настройки в прошивке:
* DCDC_Boster 1  - 0 - не используем, 1 - используем повышайку
* sleep_timer  30  - время в секундах, автоотключение
* initial_calibration 0  - вольтметр  0 - выкл, 1 - вкл (калибровка)
* battery_low 2.8  - нижний порог напряжения акб
* vape_threshold 7 - макс. время затяжки в секундах

**программа для конвертации изображений в массив:
https://sourceforge.net/projects/image2code/files/image2code/VERSION-1.1/Image2Code-1.1.zip/download
подробнее здесь:
https://www.youtube.com/watch?v=S8cbIllYc1I&ab_channel=HomeMade-%D0%A2%D0%BE%2C%D1%87%D1%82%D0%BE%D1%82%D1%8B%D0%BC%D0%BE%D0%B6%D0%B5%D1%88%D1%8C%D1%81%D0%B4%D0%B5%D0%BB%D0%B0%D1%82%D1%8C
