# Emulacja

Komputer Cobra 1 jest prostym komputerem wykorzystującym procesor Z80, który posiada klawiaturę, a funkcję monitora pełni telewizor lub monitor wyświetlający obraz o parametrach telewizyjnych 625 linii, 25Hz\. Emulator obsługuje wyświetlanie obrazu, klawiaturę, drukarkę, sygnał dźwiękowy i magnetofon\.

## Sterowanie pracą emulatora

Po uruchomieniu emulator jest w trybie zatrzymanym\. Klawisz **F1** wyświetla listę dostępnych klawiszy i udostępnia ustawienia\. Pracą steruje się za pomocą klawiszy **F5**, **F6**, **F7** i **F8**\. Naciśnięcie pierwszych dwóch klawiszy powoduje odpowiednio rozpoczęcie i zatrzymanie pracy\. Kolejne dwa klawisze to resetowanie procesora i zerowanie emulatora\. Resetowanie daje taki sam skutek, jak zresetowanie samego procesora i układu adresującego pamięć, a zerowanie dodatkowo czyści pamięć i przywraca domyślne stany wszystkich elementów\.

Po naciśnięciu klawisza **F5** emulator rozpocznie pracę i reaguje na naciskanie klawiszy\. W obszarze od C000 do C7FF znajduje się pamięć ROM, której zawartość jest wczytywana z pliku **cobra1\.rom**, domyślnie jest to oryginalne oprogramowanie rozszerzone o możliwość wprowadzanie wielkich i małych liter\. Przy zerowaniu emulatora, zerowana jest cała pamieć RAM i od adresu 0000 wczytywana jest zawartość z pliku **cobra1\.ram**, o ile istnieje\. Domyślnie, ten plik zawiera interpreter języka BASIC z wprowadzonym programem obsługi drukarki Mera\-Błonie D\-100\.

## Ekran

Komputer Cobra 1 posiada jeden tryb pracy ekranu, który jest trybem tekstowym\. Ekran wyświetla 24 linie po 32 znaki każda, sam znak ma wymiary 8x8 pikseli, wygląd znaków jest stały i nie ma możliwości programowej zmiany\. W emulatorze ekran jest siatką obrazków 8x8 pikseli, który odwzorowuje obszar pamięci od F800 do FAFF, w którym są przechowywane numery wyświetlanych znaków\. Z tego powodu nie jest możliwa emulacja zakłóceń obrazu spowodowana brakiem synchronizacji zapisu i odczytu pamięci obrazu\. W folderze z emulatorem jest plik **chr\.rom**, który jest obrazem pamięci ROM generatora znaków\. Zawartość tego pliku odpowiada wersji obsługującej semigrafikę\.

## Klawiatura

Komputer Cobra 1 posiada klawiaturę składającą się z 40 klawiszy, jednakże układ znaków specjalnych jest inny niż powszechnie przyjęty\. Emulator obsługuje wciśnięcia i puszczenia poszczególnych klawiszy, natomiast interpretacją zajmuje się uruchomione oprogramowanie\. Klawisze są podzielone na 8 grup po 5 klawiszy każda\. Odczyt stanu klawiatury polega na odczycie adresu XX80, gdzie wartość XX wskazuje wybrane grupy, przy czym w każdym bicie wybrana grupa to 0, a niewybrana grupa to 1\. W odczytanej wartości, bity od 0 do 4 wskazują naciśnięte klawisze\. Aby jednoznacznie odczytać, które klawisze zostały nacisnięte, należy odczytać kolejno adresy FE80, FD80, FB80, F780, EF80, DF80, BF80 i 7F80\. Jest to jedyna obsługiwana czynność IO, w której jest istony bardziej znaczący bajt adresy\.

Naciśnięcie klawisza **Esc** powoduje wyświetlenie okna przedstawiającego układ klawiatury i interfejs umożliwiający wysyłanie i odbieranie tekstu\.

Poniżej układu klawiatury jest lista rozwijana wyboru trybu pracy klawiatury\. Tryb **Cobra 1** to działanie klawiatury w sposób zbliżony do oryginalnej, natomiast tryb **PC** to tryb mapowania klawiszy i kombinacji do klawiszy na klawiaturze PC\. Oznacza to, że klawisz **Shift** nie powoduje emulacji naciśnięcia odpowiedniego klawisza, lecz służy do wprowadzania wielkich i małych liter oraz znaków specjalnych\.

Funkcje specjalne, sa przypisane do następujących klawiszy:


* **CAP** \(**SH**\+**T**\) \- znak **\`**, wpływa na wielkość liter przy ich wpisywaniu\.
* **CTR** \(**SH**\+**W**\) \- znak **\{**\.
* **CLS** \(**SH**\+**P**\) \- znak **\}**\.
* **SH** \- znak **\\**\.
* **CR** \- klawisz **Enter**\.

W przypadku trybu **PC**, tempo wykonywania poszczególnych czynności na emulowanym komputerze \(naciśnięcia i zwolnienia klawiszy, którym odpowiada jedna czynność użytkownika\) ustawia się w polu **Klawisze na sekundę** umieszczonym poniżej\. Wartość pola **Przerwa po Enter** nie ma znaczenia w tym przypadku\.

## Dźwięk

Emulator posiada cztery moduły generujące dźwięk:


* **Sygnalizator** \- wyzwolenie dźwięku polega na wysłaniu dowolnej wartości na adres XX18\. Istotna jest czynność wysyłania, wysyłana wartość nie ma znaczenia\. Dźwięk ma charakter impulsowy, jest odtwarzany z pliku **sound\.wav**\.
* **Generator** \- zawiera dwa kanały generujące dźwięk o kształcie prostokątnym poprzez wysyłanie ze stałą częstotliwością odpowiednich wartości, co powoduje wygenerowanie dźwięku o częstotliwości podstawowej równej połowie częstotliwości wysyłania\. Różny jest sposób wysterowania każdego konałau:
  * **Kanał 1** \- wysyłanie dowolnej wartości na adres XXFE\. Wysyłana wartość nie jest istotna, każda czynność wysłania zmienia stan przebiegu na przeciwny\. 
  * **Kanał 2** \- wysyłanie wartości na adres XX00 lub XX08 pozwala zmieniać stan generatora\. Wysłanie wartości zerowej zmienia stan na niski, a wysłanie wartości niezerowej zmienia stan na wysoki\. W przypadku wartości niezerowej, konkretna wartość nie ma znaczenia\. Wielokrotne wysyłanie wartości zerowej z rzędu lub wielokrotne wysyłanie wartości niezerowej z rzędu nie zmienia stanu generoatora\.
* **AY\-3\-8910** \- trzykanałowy generator dźwieku i szumu, zawiera 15 rejestrów, w tym 13 rejestrów związanych z generowaniem dźwięku\. Sterowanie odbywa się poprzez wysłanie numeru rejestru na adres FFFD, a następnie wysłanie wartości na adres BFFD\.

## Ustawienia

Po naciśnięciu klawisza F1 pojawia się okno z ustawieniami pracy emulatora\. Ustawienia są stosowane zaraz po zmianie wartości, ustawienia dotyczące timera i dźwięku należy zmieniać przy zatrzymanej emulacji\.

Dostępne są następujące opcje:


* **Negatyw** \- Wyświetlanie czarnych znaków na białym tle zamiast białych znaków na czarnym tle \(odwrócone kolory\)\.
* **Kolory** \- Używanie kolorów według danych zapisanych w obszarze od FC00 do FEFF\. Ta funkcjonalność nie jest dostepna w oryginalnym komputerze\.
* **Zamiana kolorów** \- Zamiana koloru tła z kolorem znaku\.
* **Wspólna pamięć F800\-FBFF i FC00\-FFFF** \- Wskazanie, że pamięć z tych dwóch zakresów ma być wspólna, co miało miejsce w pierwotnej konstrukcji Cobra1, ale uniemożliwia zastosowanie kolorowego obrazu\.
* **Częstotliwość AY\-3\-8910 \(Hz\)** \- częstotliwość zegara taktującego generator AY\-3\-8910\.
* **Okres przerwania INT \(ms\)** \- okres między dwoma wyzwoleniami przerwania maskowalnego\. Wpisanie wartości **0** spowoduje wyłączenie generowania przerwania\.
* **Głośność sygnalizatora** \- Głośność dźwięku sygnalizatora \(dźwięk odtwarzany z pliku **sound\.wav**\), zbyt duża wartość głośności może spowodować przesterowanie\.
* **Głośność generatora** \- Głośność dźwięku generatora przebiegu prostokątnego, zbyt duża wartość głośności może spowodować przesterowanie\.
* **Głośnosć AY\-3\-8910** \- Głośnośćdźwięku generatora AY\-3\-8910, zbyt duża wartość głośności może spowodować przesterowanie\.
* **Okres timera \(ms\)** \- Okres wewnętrznego timera sterującego odświeżaniem obrazu i buforowaniem dźwięku, wartość należy ustawić doświadczalnie\.
* **Długość bufora \(smp\)** \- Bufor karty dźwiękowych wyrażona w próbkach dźwięku, wartość należy ustawić doświadczalnie\.
* **Długość elementu \(smp\)** \- Długość jednego elementu buforowanego dźwięku wyrażona w próbkach, wartość należy ustawić doświadczalnie\.

# Wymiana danych

## Wczytywanie danych

W czasie pracy jest możliwe wczytywanie danych do pamięci lub w postaci naciśnięć klawiszy\. W tym celu należy nacisnąć klawisz **F2**, wybrać typ pliku i wybrać plik\. Typ **Intel HEX** to plik tekstowy z zapisanymi adresami i wartościami bajtów, wczytanie takiego pliku nie wymaga dodatkowych parametrów\. Typ **Binary** to plik, w którym kolejne bajty pliku odpowiadają wartościom kolejnych bajtów w pamięci\. Po wybraniu takiego pliku należy podać adres, od którego rozpocznie się wczytywanie\. W przypadku pliku **Intel HEX** i **Binary** możliwe jest nadpisanie zawartości pamięci ROM\.

Jest jeszcze trzeci możliwy typ, którym jest typ **Keystrokes**\. Jest to plik tekstowy, w którym są zawarte znaki znajdujące się na klawiszach\. Przy wczytywaniu zakłada się standardowy sposób obsługi klawiatury oraz, że klawisze **SH**\+**T** \(**CAP**\) przełączają pomiędzy wielkimi i małymi literami\. Wczytywanie pliku należy rozpocząć w trybie wielkich liter \(domyślny tryb pracy klawiatury\)\. Wprowadzenie jednego znaku to dwie czynności, naciśnięcie klawisza i zwolnienie klawisza\. W rzeczywistości są liczone wszystkie czynności, więc symulacja naciśnięcia **SH**\+**T** jest liczona jako cztery czynności\. Podobnie jest ze znakami specjalnymi, wczytanie takiego znaku to też cztery czynności\.

Wczytywane są wyłącznie takie znaki, które występują na klawiaturze komputera Cobra 1\. Dodatkowo, w pliku mogą występować znaki **~** oraz **\_**\. Taki znak oznacza odczekanie czasu wprowadzenia jednego znaku, użyteczne w przypadku wczytywania sekwencji naciśnięć klawiszy, w czasie których wymagane są dłuższe przerwy w określonych miejscach sekwencji ze względu na działanie programu\.

## Zapisywanie danych

Oprócz wczytywania, można tez zapisywać dane z pamięci do pliku typu **Binary** lub pliku typu **Intel Hex**\. Aby zapisać dowolny fragment pamięci, należy nacisnąć klawisz **F3**, a następnie wskazać typ pliku i podać nazwę pliku\. Niezależnie od typu pliku, w następnych dwóch okienkach należy podać adres początkowy i końcowy zapisywanego obszaru pamięci\. Sugerowany jest obszar 0000\-BFFF z tego względu, ze jest to cały obszar pamięci użytkowej przy założeniu, że w komputerze jest zainstalowana pamięć o wielkości 48kB\. Oczywiście można podać dowolne adresy z obszaru od 0000 do FFFF\.

## Wysyłanie tekstu

Po naciśnięciu **Esc** pojawia się okno zawierające interfejs służący do wysyłania i odbierania tekstu\. Aby wysłać tekst, należy wpisać lub wkleić tekst w pole tekstowe w sekcji **Wysyłanie tekstu**, a także, w razie potrzeby, dobrać wartości w polu **Klawisze na sekundę &#124; Przerwa po Enter**\. Pierwsza wartość to liczba czynności naciśnięcia lub puszczenia klawisza na sekundę, a druga wartość, to liczba okresów naciśnięcia jednego klawisza po naciśnięciu klawisza Enter\. Aby wysłać tekst, należy kliknąć przycisk **Wyślij**, wysyłanie przebiega na tej samej zasadzie, co wczytywanie tekstu z pliku tekstowego typu **Keystrokes**\. Przycisk **Wyczyść** czyści pole do wprowadzania tekstu\.

## Odbieranie tekstu

W oknie dostępnym po naciśnięciu klawisza **Esc** zawarty jest interfejs służący do odbiory tekstu\. Aby odebrać tekst, w sekcji **Odbieranie tekstu** należy wybrać odpowiednie urządzenie do symulacji w polu **Symulowane urządzenie**\. Po wybraniu urządzenia, w programie uruchomionym w emulatorze należy wydać polecenie drukujące żądany tekst\. Drukowany tekst jest dopisywany do istniejącego tekstu w polu tekstowym w sekcji **Odbieranie tekstu**\. Okno **Klawiatura i wymiana tekstu** nie musi być otwarte w czasie wysyłania tekstu, można otworzyć po zakończeniu wysyłania tekstu\. Przycisk **Wyczyść** czyście pole tekstowe\. Zajętość drukarki \(brak zdolności do odbioru następnego znaku\) i szybkość pracy drukarki nie jest symulowana, przy sprawdzaniu statusu drukarki, komputer zawsze dostaje odpowiedź interpretowaną tak, że drukarka może przyjąć następny znak\.

# Magnetofon i pamięć

Emulator jest wyposażony w symulator magnetofonu dostępny po naciśnięciu klawisza **F9**\. Za jego pomocą można korzystać ze wszystkich aplikacji korzystających z magnetofonu, a także wczytywać posiadane nagrania zgrane z kaset magnetofonowych do komputera\. Obsługiwane są wyłącznie pliki WAV \(Microsoft Wave\) zawierający dźwięk nieskompresowany \(PCM\) o dowolnej częstotliwości próbkowania \(zaleca sie 44100Hz\), monofoniczny, 16\-bit signed\. Format danych \(sposób generowania i interpretacji impulsów cyfrowych\) zależy od używanego oprogramowania używającego magnetofonu i przetwarzanie danych cyfrowych nie jest zadaniem samego magnetofonu\.

## Parametry pracy

Przed rozpoczęciem korzystania należy ustawić parametry pracy dostosowując do bieżących potrzeb\. W przypadku zmiany, nalezy ponownie wczytać nagranie lub wygenerować nowy plik i dokonać nagrania\.

Można ustawić następujące parametry:


* **WAV \- Próg rozpoczęcia impulsu** i **WAV \- Próg zakończenia impulsu** \- Przy wczytywaniu pliku WAV, wyodrębniane są impulsy cyfrowe na zasadzie analogicznej do przerzutnika Schmidta\. Są to progi wartości próbki od 0 do 32760, które określają próg rozpoczęcia impulsu i próg zakończenia impulsu\. Próg zakończenia impulsu musi być niższy od progu rozpoczęcia impulsu\. Ma to na celu zapobieżenia generowania fałszywych impulsów pod wpływem szumu i zakłóceń nagrania\.
* **Odczyt/Zapis \- Czas trwania próbki nagrania \(cykle\)** \- Liczba cykli zegara procesora na jedną próbkę nagrania podczas odtwarzania lub zapisywania\.
* **Odczyt \- Czas impulsu długiego \(próbki\)** \- Liczba próbek czasu trwania podtrzymania impulsu na przerzutniku monostabilnym, którego wyjście jest doprowadzone do układu cyfrowego komputera\.
* **Zapis \- Czas trwania impulsu \(próbki\)** \- Liczba próbek pojedynczego impulsu przy zapisie nagrania na taśmę\.
* **Zapis \- Czas do impulsu przeciwnego \(próbki\)** \- Liczba próbek pomiędzy danym impulsem a odpowiadającym mu impulsem przeciwnym w stosunku do danego\.

## Nagrywanie i odtwarzanie

Praca magnetofonu jest zsynchronizowana z pracą emulatora, z tego względu zapis i odczyt jest możliwy wyłącznie w czasie pracy emulatora, wstrzymanie pracy emulatora powoduje wstrzymanie pracy magnetofonu\.

Przed rozpoczęciem nagrywania lub odtwarzania, należy ustawić parametry pracy, a potem wygenerować nowy, pusty plik \(taśmę\) za pomocą przycisku **Nowy plik**, a następnie podać czas trwania w sekundach\. Można też wczytać istniejący plik WAV poprzez kliknięcie przycisku **Wczytaj plik** i wybranie pliku WAV\. Po dokonaniu nagrania można plik zapisać korzystając z przycisku Zapisz plik\. Zmiana długości danego pliku nie jest możliwa, jednakże w jednym pliku można przechowywać kilka nagrań lub dokonać nowego nagrania w miejscu starego tak samo, jak w przypadku rzeczywistego magnetofonu\.

Pod obrazem położenia głowicy jest pięć przycisków sterujących magnetofonem\. Te przyciski umożliwiają odpowiednio nagrywanie, odtwarzanie, zatrzymanie, przewijanie do przodu i przewijanie do tyłu\. Przewijanie to odtwarzanie z 10\-krotną prędkością bez generowania impulsów\.

## Zawartość pamięci

W oknie **Magnetofon i pamięć** można ustawiać zawartość pamięci ROM, generatora znaków i pamięci RAM bezpośrednio po zerowaniu\.

Do tego celu służą następujące pola w sekcji **Pamięć**:


* **Obraz pamięci ROM** \- Plik binarny zawierający obraz pamięci ROM, która może być w postaci pojedynczego banku lub wielu banków\.
* **Lista banków ROM** \- Plik definiujący listę banków dostępnych pod poszczególnymi numerami\. Jest to plik tekstowy zawierający dokładnie 256 linii, w każdej linii są dwie liczby zapisane w systemie szesnastkowym\. Pierwsza określa numer pierwszego bajtu danego banku, a druga określa wielkość banku\. Zaleca się, żeby wszystkie banki były tej samej wielkości\.
* **Obraz generatora znaków** \- Plik binarny zawierający wzory znaków\. Standardowo plik jest wielkości 2048 bajtów \(256 znaków po 8 bajtów na znak\), jednakże ten plik może zawierać więcej zestawów\. Liczba zestawów może wynosić 1, 2, 4, 8, 16, 32, 64, 128 lub 256 pozycji, wobec czego plik może mieć wielkość równą 2048 bajtów pomnożoną przez liczbę zestawów\. Zestawy w komputerze są numerowane od 0 do 255, przy czym w przypadku mniejszej faktycznej liczbie zestawów, zestawy powtarzają się cyklicznie\. W najprostszym przypadku, gdy zostanie wczytany plik o wielkości 2048 bajtów, to wszystkie zestawy będą takie same\.
* **Wstępna zawartość RAM** \- Plik zawierający początkową zawartość pamięci RAM po zerowaniu komputera\. Można to wykorzystać na szybkie wczytanie interpretera BASIC lub innego programu wczytywanego za pomocą magnetofonu\.

Przycisk **Wybierz plik** pozwala wybrać dowolny plik\. Jeżeli nie ma pełnej ścieżki dostępu, zostanie wczytany plik znajdujący się w folderze z emulatorem\. Aby zmiana plików odniosła skutek, po wybraniu plików należy wyzerować emulator za pomocą klawisza **F8**\.

# Rejestrowanie pracy emulatora

Emulator umożliwia rejestrowanie do pliku wykonywanie programu, co umożliwia śledzenie i analizę wykonywanego kodu\.

## Zasada działania i struktura plików

W przypadku włączonego rejestrowania pracy, rejestrowana jest każda sesja osobno, od uruchomienia emulatora \(klawisz **F5**\) do jego zatrzymania \(klawisz **F6**\)\. Włączenie lub wyłączenie opcji lub zmiana zakresu rejestrowanych informacji wykonana podczas pracy emulatora nie wpływa na biezacą rejestrację do momentu zatrzymania\.

Podczas rejestrowania pracy tworzone są dwa pliki, przy czym w miejscu znaku **X** w nazwie dopisywany jest kolejny numer:


* **progtraceX\.txt** \- Wykonane rozkazy, ewentualnie bajty pamięci i wartości rejestrów w kolejności faktycznego ich wykonania, zgodnie z przebiegiem działania programu uruchomionego w emulatorze\.
* **progmemX\.txt** \- Plik generowany po zatrzymaniu emuatora i zawierajacey rozkazy w formie kodu źródłowego i ewentualnie bajty w pamięci\. Ten plik ułatwia odtworzenie kodu asm uruchomionego programu\.

Każdy wpis w obu plikach zawiera następujące elementy:


1. Adres składajacy się z dwóch liczb:
   * Numer banku pamięci w przypadku wykonywania rozkazu z pamięci ROM lub "**\_\_**" w przypadku wykonywania programu z pamięci RAM\.
   * Numer bajtu w przestrzeni adresowej procesora Z80\.
2. Rozkaz otrzymany poprzez interpretację bajtów z pamięci\. W przypadku rozkazów skoków względnych, w miejscu adresu jest liczba poprzedzona znakiem "**\_**", która wskazuje numer bajtu, pod który jest wykonywany skok\. To ułatwia ręczne śledzenie działania programu\.
3. Bajty w pamięci, które składają się na rozkaz, element opcjonalny\.

Dodatkowo, plik **progtraceX\.txt** może opcjonalnie zawierać stany poszczególnych rejestrów w chwili poprzedzającej wykonanie danego rozkazu\.

Aby prawidłowo zarejestrować pracę, należy uruchomić i zatrzymać pracę emulatora\. Z przyczyn technicznych, zamknięcie emulatora podczas pracy może spowodować nieprawidłowe zarejestrowanie pracy wykonanej od ostatniego wystartowania do chwili zamknięcia programu\.

## Ustawienia rejestrowania pracy

Rejestrowanie pracy konfiguruje się w oknie ustawień \(klawisz **F1**\), w sekcji **Rejestrowanie pracy**\. Dostępne są następujące opcje:


* **Rejestrowanie rozkazów** \- Włącza lub wyłacza rejestrowanie wykonywanych rozkazów do plików\. Podczas rejestrowania pracy, emulacja może być spowolniona\.
* **Pamięć** \- Zapisywanie wartości bajtów z pamięci, które tworzą rejestrowane rozkazy\. Ta opcja dotyczy obu plików\.
* **Rejestry \.\.\.** \- Cztery opcje dotyczące poszczególnych grup rejestrów, dotyczą wyłącznie pliku **progtraceX\.txt**\.




