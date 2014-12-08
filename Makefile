# Makefile k projektu IMS - Studie ucelnosti zbudovani vodni cesty Dunaj-Odra-Labe
# Autori: Adam Jez <xjezad00>, Roman Blanco <xblanc01>
# Datum: 7.12.201

CC = g++
CPPFLAGS = -std=c++11 -pedantic -Wall -g -O2

.PHONY: all model
all: model

model:
	$(CC) $(CPPFLAGS) -o model model.cc parser.cc  -lsimlib -lm 

run:
	./model ship-VYSOKY-2050 & ./model ship-VYSOKY-2020 & ./model ship-NIZKY-2020 & ./model ship-NIZKY-2050 & ./model ship-TREND-2020 & ./model ship-TREND-2050

clean:
	rm -f model ./*.out
