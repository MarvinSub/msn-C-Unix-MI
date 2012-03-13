/*Desarrollado por: Rafael Oliver Murillo
		    Erick Monge Angulo 
		    Marvin Suarez Benson

este es el archivo principal del programa, el cual abrira los sockets (cliente y servidor) 
y hara las respectivas conexiones. Ademas recibe como parametros el puerto de cliente la 
ip del servidor y el puerto que utilizara el servidor, en ese orden.
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

/*Entrada--> arreglo de tipo char
  Salida---> entero que indica si el mensaje recibido fue un "adios", 
	     retorna 1 en caso de recibir un adios 0 de lo contrario.
*/
int verificar_escrito(char msj[]){
	char adios[]={'a','d','i','o','s'};
	int cierra; //1 si es igual, 0 de lo contrario
	int i;//variable para el for
	for(i=0;i<5;i++){
		if (msj[i]!=adios[i]){
			return 0;
		}
	}
	return 1;
}
/*Entrada ---> Recibe el puerto_cliente ip_servidor puerto_de_escucha
  Descripcion--> Declara las variables y estructuras necesarias(especificadas abajo), luego bifurca el proceso para poder crear
  los sockets "por separado" y asi logre levantar un servidor(proceso hijo) y un cliente(proceso padre) simultaneamente.

	En el socket servidor primero crea el socket logicamente, luego hace bind para indicar que vamos a utilizar este socket para escuchar, 
	luego aplica la funcion listen() para empezar a escuchar peticiones de cualquier cliente que se quiera conectar. en el momento que 
	"escucha" a alguien hace el accept() de la peticion, y bifurca el proceso para asi atender al cliente actual y poder seguir atendiendo
	otros sin tener que responder. Al final si el msj es un "adios" retorna un 0 y asi sale del while y cierra la conexion.

	En el socket cliente igualmente crea el socket, obtiene la direccion ip del server (argv[2]), y le asigna tanto la ip_serv como el puerto
	al que tiene que enviar. Luego entra en un ciclo que intenta conectar con el servidor, si logra conectarse sale del while o si no 		permanece en el hasta que se logre conectar, cuando logra conectarse hace una lectura del stdin, cuando lo obtiene lo manda a leer a la 	funcion verificar_escrito() y lo escribe en el socket para mandar la respuesta. Si el msj es adios cierra la comunicacion con el server.
*/
main (int argc,char *argv[]){
	
	int conexion, conexion_verif;//verificador de conexion y variable para para el ciclo cuando esta intentando conectar
	int Descriptor_serv;// Nombre del socket servidor
	int Descriptor_cli;// Nombre del socket cliente
	int aceptacion;//para verificacion cuando esta aceptando 
	int pid, pid2;//para verificar los fork
	int Puerto, Puerto_cli;//almacenaran los puertos de servidor y cliente respectivamente
	int temp, adios_ind;//temp es una variable temporal para escribir en el socket y adios_ind indica si es un adios 1 si lo es , 0 contrario.
	char msj_enviar[256];//arreglo que contiene los msjs
	int n;		
	
	struct sockaddr_in Direccion,Direccion_srv, cliente;//crea la estructura para contener las ip que se quieren atender 
	struct hostent *servidor;//estructura que contiene la ip dle server
	socklen_t Longitud_cliente;
	
		
	pid=fork();
	
	if (pid==0){ 	
			
		//se crea el socket servidor
		Descriptor_serv = socket(AF_INET, SOCK_STREAM,0);//crea el socket servidor
		if (Descriptor_serv == -1) printf ("Error\n");
		//else printf("se creo el socket servidor\n");

		bzero((char *) &Direccion, sizeof(Direccion));

		//creacion del bind del socket
		Puerto= atoi(argv[3]);//puerto por el cual escuchara el server
		Direccion.sin_family = AF_INET;
		Direccion.sin_port = htons(Puerto);// indica el puerto por el cual va a escuchar
		Direccion.sin_addr.s_addr = INADDR_ANY;//indica que atendera a cualquier ip que solicite el servicio
	
		//hace el bind  del socket Descriptor_serv con las solicitudes que entren , -1 en caso de error
		if (bind (Descriptor_serv, (struct sockaddr *) &Direccion, sizeof (Direccion))==-1)
			printf("Error en el bind\n");
		else printf("\nBienvenid@ al chat\n\n");

		if (listen(Descriptor_serv,10)==-1)//indica al servidor que empiece a escuchar solicitudes...
			printf("error listening\n");
	
		Longitud_cliente = sizeof (cliente);//verfica la longitud del msj que le enviaron
		
		while (1){
			
			aceptacion = accept(Descriptor_serv, (struct sockaddr *) &cliente, &Longitud_cliente);//
			if (aceptacion<0){
				printf("error en la aceptacion \n");
			}
			pid2=fork();
			if (pid2<0)printf("error en el pid2 \n");
			if (pid2==0){
				close(Descriptor_serv);//cierra la copia del socket creado por el fork
				adios_ind=lea_msj(aceptacion);//verifica si el msj en el accept es un adios
				if (adios_ind==1) break;
					return 0;
					exit(0);
			}
			else	close(aceptacion);			
		}//fin del while
		close(Descriptor_serv);//cierra el socket cliente
		return 0;
	}
	else{
	while(1){

		Puerto_cli =atoi (argv[1]);// este puerto es por el cual hara el conect
		
		Descriptor_cli = socket (AF_INET, SOCK_STREAM, 0);//crea el socket cliente
		if (Descriptor_cli == -1){
			printf ("error creando el socket cliente\n");		
		}
	


		servidor = gethostbyname (argv[2]);//crea la estructua que contiene la direccion ip del servidor 
		
		if (servidor == NULL){
			printf ("error en la direccion ip del servidor\n");		
		}
		bzero((char *) &Direccion_srv, sizeof(Direccion_srv));//reinicia la estructura Direccion_srv
		
		Direccion_srv.sin_family = AF_INET;//indica que el socket servira en computadores diferentes
		bcopy((char *)servidor	->h_addr, (char *)&Direccion_srv.sin_addr.s_addr,servidor->h_length);//asigna la direccion ip al socket
		Direccion_srv.sin_port = htons(Puerto_cli);//convierte el int Puerto_cli en una estructura de puerto
	

		conexion_verif=0;//indica si ya se establecio la conexion con el srv, 1 si ya lo logro, 0 de lo contrario
		while(conexion_verif==0){//termina cuando conexion_verif sea = 1
			conexion=connect (Descriptor_cli,(struct sockaddr *) &Direccion_srv, sizeof(Direccion_srv)) ;
			if(conexion<0){printf(""); }
			else {conexion_verif=1;//indica que hubo una conexion satisfactoria
				printf("\nEnviado: ");}

		}
		conexion_verif=0;
		
		bzero(msj_enviar, 256);//borra el mensaje anterior
		fgets(msj_enviar, 255, stdin);//funcion para leer del stdin el msj del usuario

		//verifica si es adios
		if(verificar_escrito(msj_enviar)==1)
			adios_ind=1;

		
		temp=write(Descriptor_cli, msj_enviar, sizeof(msj_enviar));//escribe en el socket para enviar al server
		
		if (temp<0) printf("error al escribir\n");

		temp=read(Descriptor_cli, msj_enviar, 255);

		if (temp < 0) 
			 //printf("Error al leer el socket \n");
		
		if (adios_ind == 1) return 0;//condicion de para del while externo
	}
	close(Descriptor_cli);//cierra el socket cliente
	return 0;
} 	

		return Descriptor_cli;	
	
}
/*Entrada---> entero(en realidad la estructura del accept a la hora recibir un mensaje) el cual contiene el mensaje recibido.
  Salida----> un entero, 0 en caso de no ser el mensaje leido un "adios", 1 en caso contrario.
  Descripcion---> toma el la estructura accept y le aplica la funcion read (libreria de sockets), 
		  luego de leerlo verifica que si es un adios, mediante la funcion verficar_escrito enviandole un arreglo de tipo char
*/
int lea_msj(int accept){
	int m;
	char buffer[256];
	char* scaneo;

	//bzero(buffer, 255);
	m= read(accept, buffer, 255);
	if (m<0) printf("error leyendo\n");
	printf("\n\nRecibido:%s \nEnviado: ", buffer);


	if (verificar_escrito(buffer) == 1){	   
	   return 1;
	}
   	return 0;
}	



