# Emulacja
Komputer Cobra 1 jest prostym komputerem wykorzystującym procesor Z80, który posiada klawiaturę, a funkcję monitora pełni telewizor lub monitor wyświetlający obraz o parametrach telewizyjnych 625 linii, 25Hz. Emulator obsługuje wyświetlanie obrazu, klawiaturę, drukarkę, sygnał dźwiękowy i magnetofon.

## Sterowanie pracą emulatora
Po uruchomieniu emulator jest w trybie zatrzymanym. Klawisz __F1__ wyświetla listę dostępnych klawiszy i udostępnia ustawienia. Pracą steruje się za pomocą klawiszy __F5__, __F6__, __F7__ i __F8__. Naciśnięcie pierwszych dwóch klawiszy powoduje odpowiednio rozpoczęcie i zatrzymanie pracy. Kolejne dwa klawisze to resetowanie procesora i zerowanie emulatora. Resetowanie daje taki sam skutek, jak zresetowanie samego procesora i układu adresującego pamięć, a zerowanie dodatkowo czyści pamięć i przywraca domyślne stany wszystkich elementów.

Po naciśnięciu klawisza __F5__ emulator rozpocznie pracę i reaguje na naciskanie klawiszy. W obszarze od C000 do C7FF znajduje się pamięć ROM, której zawartość jest wczytywana z pliku __cobra1.rom__, domyślnie jest to oryginalne oprogramowanie rozszerzone o możliwość wprowadzanie wielkich i małych liter. Przy zerowaniu emulatora, zerowana jest cała pamieć RAM i od adresu 0000 wczytywana jest zawartość z pliku __cobra1.ram__, o ile istnieje. Domyślnie, ten plik zawiera interpreter języka BASIC z wprowadzonym programem obsługi drukarki Mera-Błonie D-100.

## Ekran
Komputer Cobra 1 posiada jeden tryb pracy ekranu, który jest trybem tekstowym. Ekran wyświetla 24 linie po 32 znaki każda, sam znak ma wymiary 8x8 pikseli, wygląd znaków jest stały i nie ma możliwości programowej zmiany. W emulatorze ekran jest siatką obrazków 8x8 pikseli, który odwzorowuje obszar pamięci od F800 do FAFF, w którym są przechowywane numery wyświetlanych znaków. Z tego powodu nie jest możliwa emulacja zakłóceń obrazu spowodowana brakiem synchronizacji zapisu i odczytu pamięci obrazu. W folderze z emulatorem jest plik __chr.rom__, który jest obrazem pamięci ROM generatora znaków. Zawartość tego pliku odpowiada wersji obsługującą semigrafikę.

## Klawiatura
Komputer Cobra 1 posiada klawiaturę składającą się z 40 klawiszy, jednakże układ znaków specjalnych jest inny niż powszechnie przyjęty. Emulator obsługuje wciśnięcia i puszczenia poszczególnych klawiszy, natomiast interpretacją zajmuje się uruchomione oprogramowanie. Klawisze są podzielone na 8 grup po 5 klawiszy każda. Odczyt stanu klawiatury polega na odczycie adresu XX80, gdzie wartość XX wskazuje wybrane grupy, przy czym w każdym bicie wybrana grupa to 0, a niewybrana grupa to 1. W odczytanej wartości, bity od 0 do 4 wskazują naciśnięte klawisze. Aby jednoznacznie odczytać, które klawisze zostały nacisnięte, należy odczytać kolejno adresy FE80, FD80, FB80, F780, EF80, DF80, BF80 i 7F80. Jest to jedyna obsługiwana czynność IO, w której jest istony bardziej znaczący bajt adresy.

Naciśnięcie klawisza __Esc__ powoduje wyświetlenie okna przedstawiającego układ klawiatury i interfejs umożliwiający wysyłanie i odbieranie tekstu.

Poniżej układu klawiatury jest lista rozwijana wyboru trybu pracy klawiatury. Tryb __Cobra 1__ to działanie klawiatury w sposób zbliżony do oryginalnej, natomiast tryb __PC__ to tryb mapowania klawiszy i kombinacji do klawiszy na klawiaturze PC. Oznacza to, że klawisz __Shift__ nie powoduje emulacji naciśnięcia odpowiedniego klawisza, lecz służy do wprowadzania wielkich i małych liter oraz znaków specjalnych. Funkcję emulacji klawisza __SH__ przejmuje klawisz __\\__. W tym trybie można wysłać polecenie zmiany wielkości liter poprzez naciśnięcie klawisza __`__. Znaki __{__ i __}__ powodują uruchamianie kombinacji klawiszy opisanych jako __CTR__ i __CLS__. W obu trybach pracy klawiatury, klawisz __Enter__ emuluje klawisz __CR__. W przypadku trybu __PC__, tempo wykonywania poszczególnych czynności na emulowanym komputerze (naciśnięcia i zwolnienia klawiszy, którym odpowiada jedna czynność użytkownika) ustawia się w polu __Klawisze na sekundę__ umieszczonym poniżej. Wartość pola __Przerwa po Enter__ nie ma znaczenia w tym przypadku.

## Dźwięk
Wygenerowanie dźwięku polega na wysłaniu dowolnej wartości na adres XX18. Istotna jest czynność wysyłania, wysyłany znak nie jest istotny. Dźwięk ma charakter impulsowy, jest odtwarzany z pliku __sound.wav__.

Dodatkowo, możliwe jest generowanie dźwięku o kształcie prostokątnym poprzez wysyłanie dowolnej wartości na adres XXFE. Wysyłana wartość nie jest istotna, każda czynność wysłania zmienia stan przebiegu na przeciwny. Wysyłanie wykonywane ze stałą częstotliwością powoduje wygenerowanie dźwięku o częstotliwości podstawowej równej połowie częstotliwości wysyłania.

## Ustawienia
Po naciśnięciu klawisza __F1__ pojawia się okno z ustawieniami pracy emulatora. Ustawienia są stosowane zaraz po zmianie wartości, ustawienia dotyczące timera i dźwięku należy zmieniać przy zatrzymanej emulacji. Dostępne są następujące opcje:

__Czarne znaki na białym tle__ - Wyświetlanie czarnych znaków na białym tle zamiast białych znaków na czarnym tle.

__Głośność sygnalizatora__ - Głośność dźwięku sygnalizatora (dźwięk odtwarzany z pliku __sound.wav__), zbyt duża wartość głośności może spowodować przesterowanie.

__Głośność generatora__ - Głośność dźwięku generatora przebiegu prostokątnego, zbyt duża wartość głośności może spowodować przesterowanie.

__Okres timera (ms)__ - Okres wewnętrznego timera sterującego odświeżaniem obrazu i buforowaniem dźwięku, wartość należy ustawić doświadczalnie.

__Długość bufora (smp)__ - Bufor karty dźwiękowych wyrażona w próbkach dźwięku, wartość należy ustawić doświadczalnie.

__Długość elementu (smp)__ - Długość jednego elementu buforowanego dźwięku wyrażona w próbkach, wartość należy ustawić doświadczalnie.

# Wymiana danych
## Wczytywanie danych
W czasie pracy jest możliwe wczytywanie danych do pamięci lub w postaci naciśnięć klawiszy. W tym celu należy nacisnąć klawisz __F2__, wybrać typ pliku i wybrać plik. Typ __Intel HEX__ to plik tekstowy z zapisanymi adresami i wartościami bajtów, wczytanie takiego pliku nie wymaga dodatkowych parametrów. Typ __Binary__ to plik, w którym kolejne bajty pliku odpowiadają wartościom kolejnych bajtów w pamięci. Po wybraniu takiego pliku należy podać adres, od którego rozpocznie się wczytywanie. W przypadku pliku __Intel HEX__ i __Binary__ możliwe jest nadpisanie zawartości pamięci ROM.

Jest jeszcze trzeci możliwy typ, którym jest typ __Keystrokes__. Jest to plik tekstowy, w którym są zawarte znaki znajdujące się na klawiszach. Przy wczytywaniu zakłada się standardowy sposób obsługi klawiatury oraz, że klawisze SH+T przełączają pomiędzy wielkimi i małymi literami. Wczytywanie pliku należy rozpocząć w trybie wielkich liter (domyślny tryb pracy klawiatury). Wprowadzenie jednego znaku to dwie czynności, naciśnięcie klawisza i zwolnienie klawisza. W rzeczywistości są liczone wszystkie czynności, więc symulacja naciśnięcia SH+T jest liczona jako cztery czynności. Podobnie jest ze znakami specjalnymi, wczytanie takiego znaku to też cztery czynności.

Wczytywane są wyłącznie takie znaki, które występują na klawiaturze komputera Cobra 1. Dodatkowo, w pliku mogą występować znaki __~__ oraz __\___. Taki znak oznacza odczekanie czasu wprowadzenia jednego znaku, użyteczne w przypadku wczytywania sekwencji naciśnięć klawiszy, w czasie których wymagane są dłuższe przerwy w określonych miejscach sekwencji ze względu na działanie programu.

## Zapisywanie danych
Oprócz wczytywania, można tez zapisywać dane z pamięci do pliku typu __Binary__ lub pliku typu __Intel Hex__. Aby zapisać dowolny fragment pamięci, należy nacisnąć klawisz __F3__, a następnie wskazać typ pliku i podać nazwę pliku. Niezależnie od typu pliku, w następnych dwóch okienkach należy podać adres początkowy i końcowy zapisywanego obszaru pamięci. Sugerowany jest obszar 0000-BFFF z tego względu, ze jest to cały obszar pamięci użytkowej przy założeniu, że w komputerze jest zainstalowana pamięć o wielkości 48kB. Oczywiście można podać dowolne adresy z obszaru od 0000 do FFFF.

## Wysyłanie tekstu
Po naciśnięciu __Esc__ pojawia się okno zawierające interfejs służący do wysyłania i odbierania tekstu. Aby wysłać tekst, należy wpisać lub wkleić tekst w pole tekstowe w sekcji __Wysyłanie tekstu__, a także, w razie potrzeby, dobrać wartości w polu __Klawisze na sekundę | Przerwa po Enter__. Pierwsza wartość to liczba czynności naciśnięcia lub puszczenia klawisza na sekundę, a druga wartość, to liczba okresów naciśnięcia jednego klawisza po naciśnięciu klawisza Enter. Aby wysłać tekst, należy kliknąć przycisk __Wyślij__, wysyłanie przebiega na tej samej zasadzie, co wczytywanie tekstu z pliku tekstowego typu __Keystrokes__. Przycisk __Wyczyść__ czyści pole do wprowadzania tekstu.

# Odbieranie tekstu
W oknie dostępnym po naciśnięciu klawisza __Esc__ zawarty jest interfejs służący do odbiory tekstu. Aby odebrać tekst, w sekcji __Odbieranie tekstu__ należy wybrać odpowiednie urządzenie do symulacji w polu __Symulowane urządzenie__. Po wybraniu urządzenia, w programie uruchomionym w emulatorze należy wydać polecenie drukujące żądany tekst. Drukowany tekst jest dopisywany do istniejącego tekstu w polu tekstowym w sekcji __Odbieranie tekstu__. Okno __Klawiatura i wymiana tekstu__ nie musi być otwarte w czasie wysyłania tekstu, można otworzyć po zakończeniu wysyłania tekstu. Przycisk __Wyczyść__ czyście pole tekstowe. Zajętość drukarki (brak zdolności do odbioru następnego znaku) i szybkość pracy drukarki nie jest symulowana, przy sprawdzaniu statusu drukarki, komputer zawsze dostaje odpowiedź interpretowaną tak, że drukarka może przyjąć następny znak.

# Magnetofon i pamięć
Emulator jest wyposażony w symulator magnetofonu dostępny po naciśnięciu klawisza __F9__. Za jego pomocą można korzystać ze wszystkich aplikacji korzystających z magnetofonu, a także wczytywać posiadane nagrania zgrane z kaset magnetofonowych do komputera. Obsługiwane są wyłącznie pliki WAV (Microsoft Wave) zawierający dźwięk nieskompresowany (PCM) o dowolnej częstotliwości próbkowania (zaleca sie 44100Hz), monofoniczny, 16-bit signed. Format danych (sposób generowania i interpretacji impulsów cyfrowych) zależy od używanego oprogramowania używającego magnetofonu i przetwarzanie danych cyfrowych nie jest zadaniem samego magnetofonu.

## Parametry pracy
Przed rozpoczęciem korzystania należy ustawić parametry pracy dostosowując do bieżących potrzeb. W przypadku zmiany, nalezy ponownie wczytać nagranie lub wygenerować nowy plik i dokonać nagrania. Można ustawić następujące parametry:

__WAV - Próg rozpoczęcia impulsu__ i __WAV - Próg zakończenia impulsu__ - Przy wczytywaniu pliku WAV, wyodrębniane są impulsy cyfrowe na zasadzie analogicznej do przerzutnika Schmidta. Są to progi wartości próbki od 0 do 32760, które określają próg rozpoczęcia impulsu i próg zakończenia impulsu. Próg zakończenia impulsu musi być niższy od progu rozpoczęcia impulsu. Ma to na celu zapobieżenia generowania fałszywych impulsów pod wpływem szumu i zakłóceń nagrania.

__Odczyt/Zapis - Czas trwania próbki nagrania (cykle)__ - Liczba cykli zegara procesora na jedną próbkę nagrania podczas odtwarzania lub zapisywania.

__Odczyt - Czas impulsu długiego (próbki)__ - Liczba próbek czasu trwania podtrzymania impulsu na przerzutniku monostabilnym, którego wyjście jest doprowadzone do układu cyfrowego komputera.

__Zapis - Czas trwania impulsu (próbki)__ - Liczba próbek pojedynczego impulsu przy zapisie nagrania na taśmę.

__Zapis - Czas do impulsu przeciwnego (próbki)__ - Liczba próbek pomiędzy danym impulsem a odpowiadającym mu impulsem przeciwnym w stosunku do danego.

## Nagrywanie i odtwarzanie
Praca magnetofonu jest zsynchronizowana z pracą emulatora, z tego względu zapis i odczyt jest możliwy wyłącznie w czasie pracy emulatora, wstrzymanie pracy emulatora powoduje wstrzymanie pracy magnetofonu.

Przed rozpoczęciem nagrywania lub odtwarzania, należy ustawić parametry pracy, a potem wygenerować nowy, pusty plik (taśmę) za pomocą przycisku __Nowy plik__, a następnie podać czas trwania w sekundach. Można też wczytać istniejący plik WAV poprzez kliknięcie przycisku __Wczytaj plik__ i wybranie pliku WAV. Po dokonaniu nagrania można plik zapisać korzystając z przycisku Zapisz plik. Zmiana długości danego pliku nie jest możliwa, jednakże w jednym pliku można przechowywać kilka nagrań lub dokonać nowego nagrania w miejscu starego tak samo, jak w przypadku rzeczywistego magnetofonu.

Pod obrazem położenia głowicy jest pięć przycisków sterujących magnetofonem. Te przyciski umożliwiają odpowiednio nagrywanie, odtwarzanie, zatrzymanie, przewijanie do przodu i przewijanie do tyłu. Przewijanie to odtwarzanie z 10-krotną prędkością bez generowania impulsów.

## Zawartość pamięci
W oknie __Magnetofon i pamięć__ można ustawiać zawartość pamięci ROM, generatora znaków i pamięci RAM bezpośrednio po zerowaniu. Do tego celu służą następujące pola w sekcji __Pamięć__:
__Obraz pamięci ROM__ - Plik binarny zawierający obraz pamięci ROM, która może być w postaci pojedynczego banku lub wielu banków.
__Lista banków ROM__ - Plik definiujący listę banków dostępnych pod poszczególnymi numerami. Jest to plik tekstowy zawierający dokładnie 256 linii, w każdej linii są dwie liczby zapisane w systemie szesnastkowym. Pierwsza określa numer pierwszego bajtu danego banku, a druga określa wielkość banku. Zaleca się, żeby wszystkie banki były tej samej wielkości.
__Obraz generatora znaków__ - Plik binarny zawierający wzory znaków. Standardowo plik jest wielkości 2048 bajtów (256 znaków po 8 bajtów na znak), jednakże ten plik może zawierać więcej zestawów. Liczba zestawów może wynosić 1, 2, 4, 8, 16, 32, 64, 128 lub 256 pozycji, wobec czego plik może mieć wielkość równą 2048 bajtów pomnożoną przez liczbę zestawów. Zestawy w komputerze są numerowane od 0 do 255, przy czym przy mniejszej faktycznej liczbie zestawów, zestawy powtarzają się cyklicznie. W najprostszym przypadku, gdy zostanie wczytany plik o wielkości 2048 bajtów, to wszystkie zestawy będą takie same.
__Wstępna zawartość RAM__ - Plik zawierający początkową zawartość pamięci RAM po zerowaniu komputera. Można to wykorzystać na szybkie wczytanie interpretera BASIC lub innego programu wczytywanego za pomocą magnetofonu.
Przycisk __Wybierz plik__ pozwala wybrać dowolny plik. Jeżeli nie ma pełnej ścieżki dostępu, zostanie wczytany plik znajdujący się w folderze z emulatorem. Aby zmiana plików odniosła skutek, po wybraniu plików należy wyzerować emulator za pomocą klawisza __F8__.
