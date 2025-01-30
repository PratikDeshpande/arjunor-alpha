

arjunor-alpha-server: 
	g++ -o arjunor-alpha-server src/server.cpp src/resp.cpp src/eval.cpp

test:
	g++ -I h -pthread -o test tests/test_resp.cpp src/resp.cpp /usr/local/lib/libgtest.a

debug:
	g++ -o arjunor-alpha-server src/server.cpp src/resp.cpp src/eval.cpp -g

clean:
	rm -rf arjunor-alpha-server test