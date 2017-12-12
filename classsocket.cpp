/**
 * @file   classsocket.cpp
 * @Author Miguel Jiménez Gomis (alu0100970876@ull.edu.es)
 * @date   Diciembre, 2016
 * @brief  Fichero de cabecera de la clase Socket
 *
 * En este fichero se encuentra la descirpcion de la clase socket , asi como sus metodos y documentacion de los mismos
 */
#include <iostream>
#include <cerrno>		// para errno
#include <cstring>		// para std::strerror()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

struct Message {
	char username[45];
	char text[1024];
	char metadata[45];
	time_t rawtime;
};

class Socket{
	int fd = 0;//identificador del socket
	public:

		/**
    * @name Constructor
    * @brief Inicializa un objeto socket
    *
    * @param [in] address Tipo de dato sockaddr_in&
    *
    * Example Usage:
    * @code
    *   Socket A(const sockaddr_in& address);
    * @endcode
    */
		Socket(const sockaddr_in& address){//constructor
			fd = socket(AF_INET, SOCK_DGRAM, 0);//crear el socket
			if(fd < 0){//asegurarse de que el socket está correctamente creado
				std::cerr << "no se pudo crear el socket: " <<	std::strerror(errno) << '\n';
				//return 3;	// Error. Termina el programa siempre con un valor > 0
			}
			int result = bind(fd, reinterpret_cast<const sockaddr*>(&address),sizeof(address));
			if(result < 0){
				std::cerr << "falló bind: " << std::strerror(errno) << '\n';
				//return 5;	// Error. Termina el programa siempre con un valor > 0
			}
		}

		~Socket(){}//destructor

		/**
    * @name send_to
    * @brief Envia un mensaje a la direccion indicada
    *
    * @param [in] message Tipo de dato Message& (estructura que contiene el mensaje)
    * @param [in] address Tipo de dato sockaddr_in&
		*
    * Example Usage:
    * @code
    *   Objetosocket.send_to(mensaje,remoto);
    * @endcode
    */
		void send_to(const Message& message, const sockaddr_in& address){//funcion que envía mensajes
			int result = sendto(fd, &message, sizeof(message), 0,reinterpret_cast<const sockaddr*>(&address),sizeof(address));
			if (result < 0) {
				std::cerr << "falló sendto: " << std::strerror(errno) << '\n';
			//return 6;
			}
		}

		/**
    * @name receive_from
    * @brief Recive un mensaje y guarda la direccion del que envia en address y el mensaje en message
    *
		* @param [in] message Tipo de dato Message& (estructura que contiene el mensaje)
    * @param [in] address Tipo de dato sockaddr_in&
		*
    * Example Usage:
    * @code
    *   Objetosocket.receive_from(mensaje,remoto);
    * @endcode
    */
		void receive_from(Message& message,sockaddr_in& address){//funcion que recive mensajes
			socklen_t src_len = sizeof(address);
			int result = recvfrom(fd, &message, sizeof(message), 0,reinterpret_cast<sockaddr*>(&address), &src_len);
			if (result < 0) {
				std::cerr << "falló recvfrom: " << std::strerror(errno) << '\n';
				//return 8;
			}
		}
};
