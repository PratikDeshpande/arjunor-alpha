

arjunor-alpha-server: 
	g++ -o arjunor-alpha-server src/server.cpp src/response.cpp

test:
	g++ -I h -pthread -o test tests/test_response.cpp src/response.cpp /usr/local/lib/libgtest.a

debug:
	g++ -o arjunor-alpha-server src/server.cpp src/response.cpp -g

clean:
	rm -rf arjunor-alpha-server test