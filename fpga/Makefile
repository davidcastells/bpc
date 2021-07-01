all: basic_ap_uint.h 

clean:
	rm metagenerator
	rm -f basic_ap_uint.h
	rm -f /tmp/basic_ap_uint.h.metaprogram.cpp
	rm -f /tmp/basic_ap_uint.h.generator

basic_ap_uint.h: /tmp/basic_ap_uint.h.generator
	/tmp/basic_ap_uint.h.generator > basic_ap_uint.h

/tmp/basic_ap_uint.h.generator: /tmp/basic_ap_uint.h.generator.cpp
	g++ /tmp/basic_ap_uint.h.generator.cpp -o /tmp/basic_ap_uint.h.generator

/tmp/basic_ap_uint.h.generator.cpp: basic_ap_uint.h.metaprogram metagenerator
	./metagenerator basic_ap_uint.h.metaprogram > /tmp/basic_ap_uint.h.generator.cpp

metagenerator: metagenerator.cpp
	g++ metagenerator.cpp -o metagenerator


