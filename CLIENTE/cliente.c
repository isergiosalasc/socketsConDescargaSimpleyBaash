#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/unistd.h>
#include <stdbool.h>
#include <stdio_ext.h>
#include <fcntl.h>
#define GRN   "\x1B[32m"
#define RESET "\x1B[0m"
#define TAM 256

char* devuelve_nombre_ip_puerto(char argv[], const char arroba[2]);
void funcion_write(int sockfd, char buffer[], int size);
void funcion_read(int sockfd, char buffer[], int size);
int descarga_archivo(char argv[],char argv1[]);


int main( int argc, char *argv[] ) {
	/** Variables utilizadas para la configuracion del socket TCP */
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[2000];	
  	/** Variables utilizadas para el obtener el nombre usuario ip y puerto y verificar connect*/
	char ip_puerto[15];
	char usuario[15];
	char puerto1[10];
	char connectt[TAM];
	char usuario_ip_puerto[TAM];
	char ip[12];
	char user[TAM];
	/** Variables utilizadas para realizar la autenticacion de usuario*/
	bool user_autenticacion = true;
	bool password_autenticacion = false;
	bool siguiente = false;
	char buffer_men[] = "Nombre de usuario y/o contraseña incorrecto";
	int terminar = 0;
	int ok;
	bool recibe3 = true;

	//////////////////////////////////////////////////////////////////////////////////////////////

	while(1)
	{	
		setbuf(stdin,NULL);
		printf( "cliente: ");
		fgets( buffer, TAM-1, stdin );
		strcpy( connectt,devuelve_nombre_ip_puerto( buffer," " ) );
		if(!strcmp( connectt, "connect" ) )
		{
			strcpy( usuario_ip_puerto,strtok(NULL," ") );/**tengo el usuario ip puerto*/
			strcpy(usuario,devuelve_nombre_ip_puerto( usuario_ip_puerto,"@" ) );
			strcpy( user , usuario);/** guardo usuario*/	
			strcpy( ip_puerto,strtok(NULL,"@") );
			strcpy(ip,devuelve_nombre_ip_puerto( ip_puerto,":" ) );
		    strcpy(puerto1,strtok( NULL,":" ) );
		    printf("Usuario es: %s\n",user);
		    printf("Ip del servidor es: %s\n",ip);
		    printf("Puerto es: %s\n",puerto1);

		    if(!strcmp(puerto1,"6020\n") )
		    {
		    	break;
		    }
		    else
		    {
		    	printf("Comando invalido. Debe ingresar: connect usuer@127.0.0.1:6020 \n");
		    	continue;
		    }	
		    
		}
		else
		{
			printf("Comando invalido. Debe ingresar: connect usuer@ip:puerto \n");
			continue;
		}

	}
	  
	/** Guardo el puerto */
	puerto = atoi( puerto1 );
	/** creo el socket */
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 ) 
	{
		perror( "ERROR apertura de socket" );
		exit( 1 );
	}
	/** almaceno la ip del servidor */
	server = gethostbyname( ip );
	if (server == NULL) 
	{
		fprintf( stderr,"Error, no existe el host\n" );
		exit( 0 );
	}
	/** limpia la estructura */
	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	/** carga de la familia de dirreciones */
	serv_addr.sin_family = AF_INET;
	/**
	 * copia los datos del primer elemendo en el segundo con el tamaño máximo del tercer argumento. copia la IP
	 * sin_addr.s_addr: establece la IP de la máquina a la que necesitamos conectarnos.
	 */		
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	/** carga el numero de puerto */
	serv_addr.sin_port = htons( puerto );
	/** connect: nos conectamos al host connect() devuelve 0 si fue exitoso, -1 en caso contrario */
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) 
	{
		perror( "conexion" );
		exit( 1 );
	}
	while(1) 
	{
			memset( buffer, '\0', TAM );
			funcion_write( sockfd, ip, strlen(ip) );
			memset( buffer, '\0', TAM );
			funcion_read( sockfd, buffer, TAM );

		/* Realiza la autenticacion de usuario, enviado el nombre de usuario */
		if( user_autenticacion )
		{ 
			user_autenticacion = false;
			printf("\n-----Autenticacion de Usuario----\n" );
			memset( buffer, '\0', TAM );
			strcpy( buffer, user );
			printf("Usuario: %s\n", buffer);
			funcion_write( sockfd, buffer, strlen(buffer) );
			memset( buffer, '\0', TAM );
			funcion_read( sockfd, buffer, TAM );			
			printf("%s",buffer );
			password_autenticacion = true;
		}
	   /**
	 	* Realiza la autenticacion de password, enviado el password del usuario, si los datos son correctos ingresa al baash
	 	* de lo contrario sale del sistema.
	 	*/
		if( password_autenticacion )
		{
			password_autenticacion = false;
			memset( buffer, '\0', TAM );			
			fgets( buffer, TAM-1, stdin );
			funcion_write( sockfd, buffer, strlen(buffer) );			
			memset( buffer, '\0', TAM );
			funcion_read( sockfd, buffer, TAM );
			if(strcmp( buffer, buffer_men ) == 0 )
			{
				printf("%s\n", buffer );
				password_autenticacion = false;
				exit( 0 );
			}
			else
			{
				printf("%s\n", buffer );
				printf("Usted se conecto al servido baash, puede ejecutar comandos\n");
				password_autenticacion = false;
				siguiente = true;
			}
		}
	   	/** Si el usuario es correcto se estable el baash, en donde el usuario puede ingresar comando que se ejecutan en el baash */
		while( siguiente )
		{	
			recibe3 = true;
			setbuf(stdin,NULL);
			printf( GRN "\n%s@servidor-baash:~$​: " RESET, user );
			memset( buffer, '\0', TAM );
			fgets( buffer, TAM-1, stdin );
			if(!strcmp( buffer,"descarga\n" ) )
			{
				printf("Debe ingresa el archivo a descargar: descarga <nombre_archivo>\n");
				continue;
			}					
			n = write( sockfd, buffer, TAM );
			if ( n < 0 )
			{
				perror( "escritura de socket" );
				exit( 1 );
			}	
			buffer[strlen(buffer)-1] = '\0';
			if( !strcmp( "exit", buffer ) ) 
			{
				terminar = 1;
				printf( "Finalizando ejecución\n" );
				exit(0);
			}
			memset( buffer, '\0', TAM );
			while( recibe3 )
			{	
				n = read( sockfd, buffer, 2000 );
				if ( n < 0 ) 
				{
					perror( "lectura de socket" );
					exit( 1 );
				}
				if(!strcmp( "desc",buffer ) )
				{	/** funcion que realiza la descarga del archivo */
					ok = descarga_archivo(ip,puerto1); 
					if( ok )
					{
						printf("Archivo recibido correctamente... \n");
					}
					else
					{
						printf("Archivo no existe en el directorio\n" );
					}
					/** limpio buffer de teclado */
					int fdflags;
					fdflags = fcntl(STDIN_FILENO, F_GETFL, 0);
					fcntl(STDIN_FILENO, F_SETFL, fdflags | O_NONBLOCK); 
					while (getchar()!=EOF);
					fcntl(STDIN_FILENO, F_SETFL, fdflags); 
					memset( buffer, '\0', TAM );
					break;

				}
				if(strcmp("exit", buffer))
				{				
					printf("%s",buffer);
				}
				else
				{		
					recibe3 = false;
					memset( buffer, '\0', TAM );
				}

			}
			if( terminar ) 
			{
				printf( "Finalizando ejecución\n" );
				exit(0);
		    }
		}
		
	}
	return 0;
} 

/**
 * @brief: [Esta funcion retorna el nombre la ip y el puerto en distintas ocaciones]
 * 
 * @param argv [es el texto a parsear]
 * @param arroba [es el caracter hasta donde se quiere parsear]
 * @return [retorna el texto que se requiera]
 */
char* devuelve_nombre_ip_puerto(char argv[], const char arroba[2])
{
  char *token;
  token = strtok( argv, arroba );
  return token;
}

/**
 * @brief: [Escribe en el socket]
 * 
 * @param sockfd [descriptor de socket]
 * @param buffer [buffer a transmitir]
 * @param size [longitud de buffer ]
 */
void funcion_write(int sockfd,char buffer[],int size)
{
	int n;
	n = write( sockfd, buffer, size );
	if ( n < 0 ) 
	{
		perror( "escritura de socket" );
		exit( 1 );
	}
}

/**
 * @brief [Recibe los caracteres enviados al socket]
 * 
 * @param sockfd [descriptor de socket]
 * @param buffer [buffer de recepcion]
 * @param size [longitud de buffer]
 */
void funcion_read(int sockfd, char buffer[], int size)
{
	int n;
	n = read( sockfd, buffer, TAM );
	if ( n < 0 ) 
	{
		perror( "lectura de socket" );
		exit( 1 );
	}
}

/**
* @brief [Realiza la descarga del archivo en UDP]
* 
* @param argv es la ip del servidor
* @param argv1 es el puerto del servidor
* @return [retorna 1 si el archivo se recibe correctamente, retorna 0 si el archivo no se encuentra en el directorio]
*/
int descarga_archivo(char argv[],char argv1[])
{
	/** Variables utilizadas para no conexion */
	int sockfd_udp, puerto_udp, n_udp;
	socklen_t tamano_direccion_udp;
	struct sockaddr_in dest_addr_udp;
	struct hostent *server_udp;
	char filename[60] = {0};
	int capacidad = 0; 
	int nn; 
    char buff[4096] = {0};
    char tamanio[100] = {0};
	

	server_udp = gethostbyname( argv );
	if( server_udp == NULL ) 
	{
		fprintf( stderr, "ERROR, no existe el host\n");
		exit(0);
	}
	puerto_udp = atoi(argv1);
	dest_addr_udp.sin_family = AF_INET;
	dest_addr_udp.sin_port = htons( puerto_udp);
	dest_addr_udp.sin_addr = *( (struct in_addr *)server_udp->h_addr );
	memset( &(dest_addr_udp.sin_zero), '\0', 8 );
	sockfd_udp = socket( AF_INET, SOCK_DGRAM, 0 );
	if(sockfd_udp < 0) 
	{
		perror( "apertura de socket" );
		exit( 1 );
	}
	tamano_direccion_udp = sizeof( dest_addr_udp );
	n_udp = sendto( sockfd_udp, (void *)"Descarga archivo", 17, 0, (struct sockaddr *)&dest_addr_udp, tamano_direccion_udp );
	if( n_udp < 0 ) 
	{
		perror( "Escritura en socket" );
		exit( 1 );
	}
	/** Recibe el nombre del archivo */	
	if( recvfrom(sockfd_udp, (void *)filename, 60, 0, (struct sockaddr *) &dest_addr_udp, &tamano_direccion_udp ) == -1) 
    {
        perror("Can't receive filename");
        exit(1);
    }
    printf("Nombre de fichero: %s\n",filename );
	if(!strcmp("fin", filename))
    {
    	close(sockfd_udp);
      	return 0;
    }
	/** Recibe el tamaño del archivo */
    if ( recvfrom(sockfd_udp, (void *)tamanio, 60, 0, (struct sockaddr *) &dest_addr_udp, &tamano_direccion_udp ) == -1) 
    {
        perror("Can't receive filename");
        exit(1);
    }
    int cap = atoi(tamanio);
    printf("Tamaño de fichero real: %d bytes\n",cap );
    FILE *fp = fopen( filename, "wb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
 	memset( buff, 0, sizeof( buff ) );
 	/** Recibe los datos del archivo y  los guarda en el nuevo fichero*/
	while ((nn = recvfrom(sockfd_udp, (void *)buff, 4096, 0, (struct sockaddr *)&dest_addr_udp, &tamano_direccion_udp)) > 0) 
	{
		capacidad = capacidad + nn;
        if (nn == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
	    fwrite(buff, sizeof(char),nn, fp);
        if( cap == capacidad || !strcmp("exit",buff))
		{				
			break;
		}
        memset(buff, '\0', 4096); 
	}
	printf("Tamaño de fichero descargado: %d bytes\n",capacidad );
	fclose(fp);
	close(sockfd_udp);
	return 1;		
}

