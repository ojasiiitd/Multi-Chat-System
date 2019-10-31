all: server client
	@echo "> Successful."

server: 
	@echo "> Builing Server..."
	@gcc -pthread -o s server.c
	@echo "> Server built."

client:
	@echo "> Building Client..."
	@gcc -pthread -o c client.c
	@echo "> Client built."

clean:
	@echo "> Cleaning..."
	@rm s c
	@echo "> Done Cleaning."