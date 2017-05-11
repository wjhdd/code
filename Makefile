.PHONY:all
all:http
	cd cgi_bin;make;cd -
http: /home/admin/class/4-1/main.o /home/admin/class/4-1/http.o
	gcc -o $@ $< -lpthread
%\.o:%.c
	gcc -c $<
.PHONY:output
output:
	mkdir output
	cp -rf log output/
	cp -rf conf output/
	cp http output/
	cp -rf wwwroot output/
	cp http_ctl.sh output/
	mkdir -p output/wwwroot/cgi_bin
	cp cgi_bin/cgi_math output/wwwroot/cgi_bin
.PHONY:clean
clean:
	rm -rf http *.o output;cd cgi_bin;make clean;cd -
