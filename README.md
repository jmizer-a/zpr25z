# zpr25z

## Zależności

### Ubuntu 24.04
```bash
sudo apt install build-essential cmake qt6-base-dev libarmadillo-dev libmlpack-dev libensmallen-dev clang-tidy clang-format pkg-config
```
Zależności można również zainstalować poprzez uruchomienie skryptu `install_dependencies.sh`.

## Budowanie

Po zainstalowaniu zależności należy użyć polecenia:
```bash
cmake -B build && cmake --build build
```
Lub w przypadku korzystania z edytora VSCode można użyć przycisku `Build` (po zainstalowaniu odpowiednich rozszerzeń).

## Uruchamianie

Po zbudowaniu projektu w katalogu `build` pojawią się dwa pliki wykonywalne: `MonopolyGame` oraz `MonopolyTrain` (oraz plik wykonywalny testów jednostkowych `UnitTests` w podkatalogu `tests`). `MonopolyGame` uruchamia interfejs graficzny gry, natomiast `MonopolyTrain` uruchamia uczenie AI.

## Testy jednostkowe

Testy jednostkowe obejmują logikę gry.

## Standardy kodowania

Standardy kodowania zapewnione i egzekwowane przez editorconfig, clang-format oraz clang-tidy. Zdecydowaliśmy się na jedno (w naszej opinii nieznaczne) odstępstwo od stylu kodowania w postaci umieszczenia spacji przed nawiasami klamrowymi. Wynika to z faktu, że clang-format nie udostępnia opcji konfiguracyjnej pozwalającej na takie sformatowanie kodu. Postanowiliśmy więc zrezygnować z tego jednego aspektu stylu kodowania na rzecz automatycznego formatowania kodu przez clang-format.

