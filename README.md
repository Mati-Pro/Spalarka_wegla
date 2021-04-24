Autor: Paweł Rogóż
Kraków 2021
Hardware: Atmega328

Projekt sterownika kotła automatycznego z podajnikiem ślimakowym z dodatkową obsługą mieszacza, pomp CO, CWU i podłogówki.
Sterownik umożliwia komunikacje po RS232 z wyniesionym pilotem jako zdalnego zadajnika parametrów oraz rejestratorem
parametrów spalania na karte SD (too doo: wyniesony pilot).

Opis plików SRC:

main - inicjacja układu i pętla głowna,

mainPRG - program główny - realizacja funkcji MENU i ekranu głównego, procedury obsługi zegara RTC DS1307,

sys1Wire - biblioteka obsługi czujników DS18B20 OneWire,

sysClock - bibliotega zegara systemowego (SysTick) - realizacja na liczniku T2 z wykorzystaniem przerwań,

sysEnc - bibliotega obsługi enkodera rotacyjnego z uwzględnieniem "debouncing" i przerwań. Przeznaczenie: poruszanie się po Menu i nastawa parametrów,

sysLCD - bibliotega obsługi LCD 20x4. Komunikacja po I2C z wykorzystanie kolejkowania danych w buforze FIFO,

sysTWI - biblioteka I2C z wykorzystaniem kolejkowania w buforze FIFO z wykorzystaniem przerwań,

sysSPI - procedury obsługi MAX6675 po SPI,

USART - bibliotega obsługi transmisji asynchronicznej bez wykorzystania przerwań. W chwili obecnej wykorzystywana do debagowania.
