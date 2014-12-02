
Manuál

Přeložení zdrojových kódů

Pro překlad je využíván multiplatformní nástroj qmake (http://qt-project.org/doc/qt-5/qmake-manual.html). Tento nástroj automaticky generuje soubory Makefile pro platformu, na které je prováděn překlad. Pro vývoj aplikací je vhodné používat Qt Creator, který automaticky vygeneruje pomocí qmake Makefile a následně aplikaci přeloží. Při použití příkazové řádky např. na systému linux stačí provést v umístění projektového souboru (*.pro) příkaz qmake a poté make.

hdf5file
Pomocná knihovna pro práci s HDF5 soubory, je vyžadována pro překlad aplikací k-wave-h5-processing a k-wave-visualizer. Cesty ke knihovně hdf5file jsou nastaveny souboru "/hdf5file_paths.pri". Pro překlad knihovny hdf5file je potřeba nastavit správné cesty (INCLUDEPATH, LIBS a QMAKE_LFLAGS += -Wl,-rpath,/... v Linuxu) ke knihovně HDF5 v souboru "/hdf5_paths.pri"

k-wave-h5-processing
Projektový soubor konzolové aplikace (k-wave-h5-processing.pro) se nachází ve složce "/k-wave-h5-processing". Pro překlad konzolové aplikace je nutné nastavit správné cesty (INCLUDEPATH, LIBS a QMAKE_LFLAGS += -Wl,-rpath,/...) ke knihovně OpenCV v souboru "/opencv_paths.pri". Aplikace se přeloží do adresáře "/build/k-wave-h5-processing".

k-wave-visualizer
Projektový soubor grafické aplikace (k-wave-visualizer.pro) je umístěn v adresáři "/k-wave-visualizer". Pro překlad grafické aplikace je vyžadována verze Qt 5.1 nebo vyšší a je nutné nastavit správné cesty (INCLUDEPATH, LIBS a QMAKE_LFLAGS += -Wl,-rpath,/...) ke knihovně OpenCV v souboru "/opencv_paths.pri". Aplikace se přeloží do adresáře "/build/k-wave-visualizer".

Použití aplikací

Použití konzolové aplikace je popsáno v nápovědě při spuštění. Aplikace k-wave-h5-processing.exe je pro systém Windows přeložena s potřebnými knihovnami staticky, nepotřebuje tedy žádné pomocné soubory *.dll.

Po spuštění grafické aplikace se zobrazí hlavní okno a pro výpis pomocných informací se na pozadí spustí konzole. Dokovatelné panely lze vyjmout z hlavního okna a měnit jejich velikost. Nástrojový panel obsahuje zleva tlačítka pro:
- otevření HDF5 souboru,
- zavření souboru,
- zobrazení/skrytí panelů pro řezy,
- zapnutí/vypnutí volume renderingu,
- zobrazení/skrytí řezů v 3D zobrazení,
- zobrazení/skrytí 3D rámečku,
- roztáhnutí datasetu definovaného senzorovou maskou,
- 6 tlačítek pro zarovnání 3D scény do hlavních pohledů,
- tlačítko pro export obrázku z 3D scény
- a tlačítka pro zobrazení/skrytí panelů pro: seznam datasetů, informace o HDF5 souboru a nastavení zobrazení datasetu. 

Každý panel pro 2D zobrazení řezu obsahuje:
- slider pro změnu polohy řezu,
- tlačítka pro přepínaní mezi originální a přizpůsobenou velikostí,
- a tlačítko pro export obrázku.

V nastavení zobrazení vybraného datasetu lze:
- měnit spektrum pro mapování barev,
- upravovat maximální či minimální hodnoty, podle kterých je mapování prováděno,
- zapnout nebo vypnout zobrazení hodnot přesahujících zadaný rozsah
- a pro jednotlivé řezy lze povolit lokální změny minimálních a maximálních hodnot. 

Panel dále obsahuje ovládání animace pro časově proměnné série datasetů, kde lze:
- spustit nebo zastavit animaci,
- přeskočit na začátek či konec série,
- ručně měnit aktuální krok
- a měnit časový interval a inkrement pro změnu kroků při animaci.

Volume rendering lze ovlivnit změnou:
- průhlednosti (Alpha) zarovnaných řezů, 
- zastoupení jednotlivých barevných složek (Red, Green a Blue)
- a počtu vykreslených řezů (Slices) - kvalita volumetrického zobrazení.

Ovládání 3D scény
- Rotaxe scény - Držením levého tlačítka myši a pohybem myší.
- Přiblížení/oddálení scény - Rotací kolečka myši.
- Posun scény - Držením pravého tlačítka myši a pohybem myší.
- Reset pozice scény - Kliknutí prostředního tlačítka (kolečka) myši.



