/**
 * @file   main.cpp
 * @Author Miguel Jiménez Gomis (alu0100970876@ull.edu.es)
 * @date   Diciembre, 2016
 * @brief  Fichero principal del programa Talk
 *
 * En este fichero se encuentra el main del programa, asi como las funciones necesarias para su correcto funcionamiento ya sea en modo cliente o servidor
 */
#include "classsocket.cpp"
#include <thread>
#include <pthread.h>
#include <atomic>
#include <vector>
#include <stdio.h>
#include <fstream>

bool flagrecivir = 1;//flag para indicar si se debe terminar la ejecucion
std::vector<sockaddr_in> direcciones;//vector de direcciones conectadas al server
std::vector<Message> mensajes;//vector que funciona como cola de mensajes

/**
* @name clear_array
* @brief "funcion que limpia el array de char de tamaño 1024"
*/
clear_array(char mensaje[]){
	for(int i = 0; i < 1024;i++){
		mensaje[i] = '\0';
	}
}

/**
* @name clear_message
* @brief uncion que limpia el array de char de tamaño 1024 del atributo de message
*/
clear_message(Message& message){
	for(int i = 0; i < 1024;i++){
		message.text[i] = '\0';
	}
}

/**
* @name copy_string
* @brief funcion que copia el contenido de un string en un char(tamaño 1024)
*/
copy_string(char mensaje[],std::string& tmp){
	if(tmp.size()>=1024){
		for(int i = 0; i < 1024; i++){
			mensaje[i] = tmp[i];
		}
	}
	else{
		for(int i = 0; i < tmp.size(); i++){
			mensaje[i] = tmp[i];
		}
	}
}

/**
* @name copy_message
* @brief funcion que copia un campo text de Message en un array char
*/
copy_message(Message& message,char mensaje[]){
	for(int i = 0; i < 1024; i++){
			message.text[i] = mensaje[i];
	}
}

/**
* @name make_ip_address
* @brief funcion que retorna un socket address completo
*
* @param [in] ip_address string& (direccion ip formato X.X.X.X)
* @param [in] port int que indica a que puerto se quiere conectar
*/
sockaddr_in make_ip_address(const std::string& ip_address, int port){
	sockaddr_in local_address{};	// Porque se recomienda inicializar a 0
	local_address.sin_family = AF_INET;
	//local_address.sin_addr.s_addr = htonl(ip_address);
	local_address.sin_port = htons(port);
	if(ip_address.size() == 0){
		local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else{
		inet_aton(ip_address.c_str(),&local_address.sin_addr);
		}
		return local_address;
};

/**
* @name help
* @brief funcion de ayuda al usage
*/
void help(void){
	std::cout << "\nUsage:\n-h ----> help\n";
	std::cout << "-s -p 'Numerodepuerto'---->establecer servidor escuchando ese puerto puerto\n";
	std::cout << "-c 'DirecccionIP' -p 'Numerodepuerto'----> establece una conexion de cliente con el servidor de IP y puerto indicado\n";
}

/**
* @name mensajesincliente
* @brief funcion del hilo de recivir mensajes, escucha continuamente un puerto y muestra los mensajes
*
* @param [in] Socket& Local objeto socket con el que se hace la llamada
* @param [in] sockaddr_in& remoto Dato donde se guarda la direccion del dato recivido
* @param [in] ofstream& historial ostream de salida estandar para volcar los datos en el historial
*/
void mensajesincliente(Socket& Local,sockaddr_in& remoto,std::ofstream& historial){//funcion para el hilo de recibir mensajes
	while(flagrecivir){
		Message message;
		clear_message(message);
		Local.receive_from(message,remoto);
		char* dt = ctime(&message.rawtime);
		if(strcmp(message.metadata, "login") == 0){
			std::cout << "El usuario " << message.username << " ha entrado en la sesion" << "\t\t--" << dt <<"\n";
			historial << "El usuario " << message.username << " ha entrado en la sesion"<< "\t\t--" << dt <<"\n";
		}
		else if(strcmp(message.metadata, "logout") == 0){
			std::cout << "El usuario " << message.username << " ha salido de la sesion"<< "\t\t--" << dt <<"\n";
			historial << "El usuario " << message.username << " ha salido de la sesion"<< "\t\t--" << dt <<"\n";
		}
		else{
			std::cout << "$->" << message.username << " -> " << message.text << "\t\t--" << dt <<"\n";
			historial<< "$->" << message.username << " -> " << message.text << "\t\t--" << dt <<"\n";
		}
	}
}

/**
* @name mensajesoutcliente
* @brief Envia al servidor indicado los mensajes que introduzca el usuario(para el hilo de enviar mensajes)
*
* @param [in] Socket& Local objeto socket con el que se hace la llamada
* @param [in] sockaddr_in& remoto Direccion del server
* @param [in] char nombre[] paso por referencia del username
*/
void mensajesoutcliente(Socket& Local,sockaddr_in& remoto,char nombre[]){//funcion para el hilo de enviar mensajes
	char mensaje[1024];
	std::string tmp;
	std::getline(std::cin,tmp);
	clear_array(mensaje);
	copy_string(mensaje,tmp);
	do{
		if(strcmp(mensaje, "/quit") != 0){
				Message message;
				for(int i = 0; i < 45; i++){
					message.username[i] = nombre[i];
				}
				for(int i = 0; i < 45; i++){
					message.metadata[i] ='\0';
				}
				copy_message(message,mensaje);
				time (&message.rawtime);
				Local.send_to(message,remoto);
				clear_message(message);
				clear_array(mensaje);
				tmp.clear();
				std::getline(std::cin,tmp);
				copy_string(mensaje,tmp);
		}
	}while(strcmp(mensaje, "/quit") != 0);
	Message message;
	for(int i = 0; i < 45; i++){
		message.username[i] = nombre[i];
	}
	for(int i = 0; i < 45; i++){
		message.metadata[i] ='\0';
	}
	tmp = "logout";
	for(int i = 0; i < 6; i++){
		message.metadata[i] = tmp[i];
	}
	time (&message.rawtime);
	Local.send_to(message,remoto);
	flagrecivir = 0;
}

/**
* @name modocliente
* @brief Funcion que controla los hilos del programa para la version de cliente
*
* @param [in] char ip[] direccion ip del servidor en formato X.X.X.X
* @param [in] int puerto indicador del puerto del servidor
*/
void modocliente(char ip[], int puerto){
	std::cout << "Bienvenido a TALK" << '\n';
	std::cout << "----------------------------------------------------------------------" << '\n';
	char username[45];
	Message login;
	for(int j = 0; j <45; j++){
		username[j] = '\0';
	}
	std::string tmp;
	std::cout << "\nCual es su nombre de usuario?" << '\n';//solicitud de nombre de usuario
	do{
		std::getline(std::cin,tmp);
		if(tmp == "Admin"){//asegurarse de que no coincida con el nombre del administrador
			std::cout << "Elige otro nombre de usuario" << '\n';
		}
	}while(tmp == "Admin");
	std::cout << "----------------------------------------------------------------------" << '\n';
	if(tmp.size() > 45){
		for(int j = 0; j < 45; j++){
			username[j] = tmp[j];
			login.username[j] = tmp[j];
		}
	}//se introduce el username en el mensaje de login
	else{
		for(int j = 0; j < tmp.size(); j++){
			username[j] = tmp[j];
			login.username[j] = tmp[j];
		}
	}
	sockaddr_in local = make_ip_address("",0);
	Socket A(local);
	sockaddr_in remoto = make_ip_address(ip,puerto);//se crean las direcciones y el socket
	for(int j = 0; j < 45; j++){//se limpia el mensaje
		login.metadata[j] ='\0';
	}
	for(int j = 0; j < 1024; j++){
		login.text[j] = '\0';
	}
	std::string dato("login");//se le indica con el metadata que es un mensaje de login
	for(int j = 0; j < 6; j++){
		login.metadata[j] = dato[j];
	}
	time (&login.rawtime);
	A.send_to(login,remoto);//se envia el mensaje al server
	std::ofstream historial;
	std::string tmp2(".log");
	tmp += tmp2;
	historial.open(tmp.c_str(),std::ofstream::app);//se abre el historial
	//hilo de recivir mensajes
	std::thread hilorecivir(&mensajesincliente, std::ref(A), std::ref(remoto),std::ref(historial));
	//hilo de enviar mensajes
	std::thread hiloenviar(&mensajesoutcliente, std::ref(A), std::ref(remoto), std::ref(username));
	while(flagrecivir){}//el programa se queda aqui hasta que se escriba "/quit"
	pthread_cancel(hiloenviar.native_handle());//se eliminan los hilos
	pthread_cancel(hilorecivir.native_handle());
	hiloenviar.join();//se espera por los hilos
	hilorecivir.join();
	std::cout << "----------------------------------------------------------------------" << '\n';
	historial << "----------------------------------------------------------------------" << '\n';
	std::cout << "Hasta la próxima " << username <<  '\n';
	historial.close();
}

/**
* @name serveradmin
* @brief Metodo para el hilo de control del administrador del servidor
*
* @param [in] Socket que realiza la llamada
*/
void serveradmin(Socket& A){
	Message aviso;
	std::string tmp;
	tmp = "Server offline";
	clear_message(aviso);
	time (&aviso.rawtime);
	for(int i = 0; i < 1024; i++){
		if(i<14){
			aviso.text[i] = tmp[i];
		}
		else
			aviso.text[i] = '\0';
	}
	tmp = "Admin";
	for(int i = 0; i < 45; i++){
		if(i<5){
			aviso.username[i] = tmp[i];
		}
		else
			aviso.username[i] = '\0';
	}
	std::string accion;
	do{
		std::cout << "\nIntroduzca la accion \n";
		std::cin >> accion;
		if(accion ==  "-h"){
			std::cout << "\n/quit para salir" << '\n';
			std::cout << "-h para ayuda" << '\n';
		}
	}while (accion != "/quit");
	for(int i = 0; i < direcciones.size(); i++){
		A.send_to(aviso,direcciones[i]);
	}
	flagrecivir = 0;
}

/**
* @name mensajesoutserver
* @brief vacia constantemente la cola de mensajes y los va enviando a todos los usuarios conectados
*
* @param [in] Socket que realiza la llamada
*/
void mensajesoutserver(Socket& A){
	while (flagrecivir) {
		while (mensajes.size() != 0){
			for(int i = 0; i < direcciones.size();i++){
				A.send_to(mensajes[mensajes.size()-1],direcciones[i]);
			}
			mensajes.pop_back();
		}
	}
}

/**
* @name mensajesinserver
* @brief recive los mensajes de los clientes, los añade a la cola, y los muestra por pantalla
*
* @param [in] Socket que realiza la llamada
*/
void mensajesinserver(Socket& A){
	while (flagrecivir) {
		sockaddr_in remoto = make_ip_address("",0);
		Message message;
		A.receive_from(message,remoto);
		std::vector<Message>::iterator it;
		it = mensajes.begin();
		if(strcmp(message.metadata, "login") == 0){
			direcciones.push_back(remoto);
			mensajes.insert(it,message);
		}
		else{
			mensajes.insert(it,message);
		}
		char* dt = ctime(&message.rawtime);
		if(strcmp(message.metadata, "login") == 0){
			std::cout << "El usuario " << message.username << " ha entrado en la sesion" << "\t\t--" << dt <<"\n";
		}
		else if(strcmp(message.metadata, "logout") == 0){
			std::cout << "El usuario " << message.username << " ha salido de la sesion"<< "\t\t--" << dt <<"\n";
		}
		else{
			std::cout << "$->" << message.username << " -> " << message.text << "\t\t--" << dt <<"\n";
		}
	}
}
/*
		debe estar escuchando todo el rato
		primero mira el  metadata, si es login o logout inserta una mensaje con el nombre de usuario y el metadata correspondiente
		cada ec¡z que le llega un mensaje mira quien lo manda
		si ya esta en la lista de direcciones inserta el mensaje en la cola de mensajes
		si no esta, añade la direccion a la lista de mensajes y lo inserta en la cola
*/

/**
* @name modoservidor
* @brief Funcion que controla los hilos del programa para la version de servidor
*
* @param [in] int puerto indicador del puerto del servidor
*/
void modoservidor(int puerto){
	sockaddr_in local = make_ip_address("",puerto);
	Socket A(local);
	std::thread hiloaccionadmin(&serveradmin, std::ref(A));
	std::thread hilorecivir(&mensajesinserver, std::ref(A));
	std::thread hiloenviar(&mensajesoutserver, std::ref(A));
	while(flagrecivir){}//el programa se queda aqui hasta que se escriba "/quit"
	pthread_cancel(hiloenviar.native_handle());//se eliminan los hilos
	pthread_cancel(hilorecivir.native_handle());
	pthread_cancel(hiloaccionadmin.native_handle());
	hiloenviar.join();//se espera por los hilos
	hilorecivir.join();
	hiloaccionadmin.join();
}

/**
* @name main
* @brief Programa principal del Talk, controla las opciones
*/
int main(int argc, char* argv[]){
	if(argc == 2){
			help();
	}
	else if(argc == 4){//modo servidor
		int i;
		sscanf(argv[3], "%d", &i);//para pasar a int no se si funciona
		modoservidor(i);//como parametro se le tiene que dar la posicon 4 pasado a entero
	}
	else if(argc == 5 || argc == 6){//modo cliente
		int i;
		sscanf(argv[4], "%d", &i);//para pasar a int no se si funciona
		modocliente(argv[2],i);//como parametro se le tiene que dar la posicon 3(como char array) y la  5 pasado a entero
	}
	else{
		std::cout << "Error en la introduccion de opciones" << std::endl;
		help();
	}
}
