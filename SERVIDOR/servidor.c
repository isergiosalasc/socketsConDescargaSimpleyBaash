#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <netdb.h> 
#include <pwd.h>	
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>	
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#define BUFFERSIZE 256										
#define MAX 51		
#define TAM 256

/** funciones para leer o escribir el socket */
void funcion_write(int newsockfd, char buffer[], int size);
void funcion_read(int newsockfd, char buffer[], int size);
/** funciones para realizar el parseo y la descarga de archivo en udp*/
char* devuelve_parseo(char argv[], const char arroba[]);
int descarga_archivo(int puerto, char buffer[]);

/** funciones del baash */
void MostrarInfoAutor();									
void Cabecera();											
int leeComando(char *argv[], char *cadena);					
void buscaPaths(char *paths[]);								
void buscaArchivo(char *arch,char *paths[],char *execPath);	


int main( int argc, char *argv[] ) {
	/** Variables utilizadas para la configuracion del socket TCP */
	int sockfd, puerto, pid;
	socklen_t clilen;
	char buffer[TAM];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	/** Variables utilizadas para realizar la autenticacion de usuario */
	bool user_autenticacion = true;
	bool password_autenticacion = false;
	bool writee1 = false;
	bool writee = false;
	bool siguiente = false;
	/** Variables utilizadas para realizar el envio de ficheros, leer la ip   */
	bool baash = false;
	char buffer_aux[TAM];
	bool ficheroo = false;
	char descarga[TAM];
	char fichero[TAM];
	int envio;
	char id_global_user[5];
	char ip[20];
	/** Variable utilizada para enviar la salida por pantalla*/
	char salida[2000] = "";	
	/** Variables de control de parseo para baash */
	char *argV[20];									
	char comando [BUFFERSIZE];
	/** Guardamos el hostname de la maquina */
	char hostname[BUFFERSIZE];								
	gethostname(hostname,sizeof(hostname));					
	/** Guardamos el nombre de usuario */
	char *user; 											
	struct passwd *pass; 									
	pass = getpwuid(getuid()); 								
	user = pass->pw_name;									
	/** Guardamos ruta completa del directorio actual */
	char dirActual[BUFFERSIZE];								
	getcwd(dirActual,sizeof(dirActual));					
	/** Mostramos directorios de path  y guardamos los directorios del pathb*/
	char executePath[BUFFERSIZE]="";						
	char *paths[BUFFERSIZE];

	//////////////////////////////////////////////////////////////////////////////////////////////

	printf("------SERVIDOR CONECTADO------\n");
	/** creo el socket */
	sockfd = socket( AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 ) { 
		perror( " apertura de socket ");
		exit( 1 );
	}
	/** limpieza de la estructura */
	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = 6020 ;
	/** carga de la familia de direcciones */
	serv_addr.sin_family = AF_INET;
	/**
	 * INADDR_ANY: También es posible asociar direcciones IP sin tener que averiguar su valor
	 * máquina con múltiples interfaces de red permite que su servidor recibiera paquetes destinados a 
	 * cualquiera de las interfaces.
	 */
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	/** Carga el numero de puerto */
	serv_addr.sin_port = htons( puerto );
	/**
	 * Bind: asignar un socket a un puerto A partir de aquí, se ha relacionado el
	 * proceso en ejecución con la dirección correspondiente y se está en condiciones
	 * de recibir datos
	 */
	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 )
	{
		perror( "ligadura" );
		exit( 1 );
	}
	printf( "Proceso: %d - Socket TCP disponible en puerto: %d\n", getpid(), ntohs(serv_addr.sin_port) );
	/** Listen poner a la escucha un socket */
	listen( sockfd, 5 );
	clilen = sizeof( cli_addr );

	while( 1 )
	{
		int newsockfd;
		/**
		 * función accept que nos servirá para estar a la escucha y permitir que algún cliente se 
		 * conecte tras utilizar la función connect()
		 */
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );//para el proceso hijo
		if ( newsockfd < 0 ) 
		{
			perror( "accept" );
			exit( 1 );
		}
		pid = fork(); 
		if ( pid < 0 ) 
		{	
			perror( "fork" );
			exit( 1 );
		}
		/** Proceso hijo */
		if ( pid == 0 )
		{  /*La función close() cierra el descriptor de fichero que le pasemos como parámetro. Devuelve 0 en caso
			 de éxito y -1 en caso de error.*/
		    close( sockfd );
			while ( 1 )
			{	
				memset( buffer, 0, TAM );
				funcion_read( newsockfd, buffer ,TAM-1);				
				printf("ip es: %s\n",buffer );
				strcpy( ip,buffer );				
				funcion_write(newsockfd,"Ok", 3);
				/** Autenticacion del usuario primero mediante el socket recibe el nombre de usuario */
				if( user_autenticacion )
				{				
					printf("\n-----Autenticacion de Usuario----\n" );
					user_autenticacion = false;
					memset( buffer, 0, TAM );
					funcion_read( newsockfd, buffer ,TAM-1);
					printf("Usuario conectado: %s\n", buffer );
					/** Abro el fichero para leer los usarios disponibles*/
				    FILE *fp_u;
				    char us[60];
				    char id_u[5];
				    char usuario[60];
   				    /** Abro fichero para leer */
				    fp_u = fopen("usuario.txt" , "r");
				    if(fp_u == NULL) 
				    {
				     	perror("Error opening file");
				      	return(-1);
				   	}				   				   	
				   	/** lee el fichero para saber si esl usaurio esta registrado */				   	
				   	while( fgets (us, 60, fp_u)!=NULL ) 
				   	{
				       	strcpy(usuario,devuelve_parseo(us," "));
				     	strcpy( id_u,strtok(NULL," ") );	
				      	if(strcmp( usuario, buffer ) == 0 )
						{
							strcpy(id_global_user,id_u);
							break;
						}
						else
						{
							strcpy(id_global_user,"0");
						}
				   }
				   fclose(fp_u);				   

				   funcion_write(newsockfd,"Ingrese su contraseña: ", 24);
				   password_autenticacion = true;
			  	}
				/**
				 * Autentica la contraseña del usuario si esta en el fichero de contraseñas, el usuario ingresa
				 * al sistema de lo contrario se le denega el acceso 
				 */
				if( password_autenticacion )
				{	
					memset( buffer, 0, TAM );
					funcion_read(newsockfd, buffer, TAM-1 );
					FILE *fp_p;
				    char pass[60];
				    char id_p[5];
				    char id_global_pass[5];
				    char passw[60];
				    buffer[strlen(buffer)-1] = '\0';
				    /** Abre ficheros de contraseñas */
				    fp_p = fopen("password.txt" , "r");
				    if(fp_p == NULL) 
				    {
				     	perror("Error opening file");
				      	return(-1);
				   	}				   
				   	while( fgets (pass, 60, fp_p)!=NULL ) 
				   	{
				   				   		
					   	strcpy(passw,devuelve_parseo(pass," "));
				     	strcpy( id_p,strtok(NULL," ") );
	
				      	if(strcmp( passw, buffer ) == 0 )
						{
							strcpy(id_global_pass,id_p);
							break;
						}
						else
						{
							strcpy(id_global_pass,"0");
						}
				   	}
				   	fclose(fp_p);
				/**Para siguiente actualizacion se pretende terminar el proceso de autenticacion. */
	        		/** Si el usaurio esta registrado writee1  es true acceso correcto, de lo contrario writee es true acceso falso */
  				   	if(1)
				   	{
				   		writee1 = true;
				   	}
				   	else
				   	{
				   		writee = true;	
				   	}
					if( writee1 )
					{
						printf( "Servidor: Usuario y Contraseña correcto\n" );
						writee1 = false;
						password_autenticacion = false;
						funcion_write(newsockfd, "\n---Usuario y Contraseña correcto---", 50);
						siguiente = true;
						printf("\n----------------BIENVENIDO AL BAASH----------------\n");
						MostrarInfoAutor();											
						buscaPaths(paths);
					}

					if( writee )
					{
						writee = false;
						password_autenticacion = false;
						printf( "Servidor: nombre de usuario y/o contraseña incorrecto\n" );
						funcion_write( newsockfd, "Nombre de usuario y/o contraseña incorrecto", 44 );
						exit( 0 );
					}
				}		

				while( siguiente )
				{
					memset( buffer, 0, TAM );
					funcion_read( newsockfd, buffer, TAM);
					baash = true;
					strcpy( buffer_aux,buffer );
					strcpy( descarga,devuelve_parseo( buffer_aux," " ) );
					/** Si se recibe descarga se realiza la descarga en modo UDP*/
					if(!strcmp( descarga,"descarga" ) )
					{	
						strcpy( fichero,strtok(NULL," ") );
						ficheroo = true;
						fichero[strlen(fichero)-1] = '\0';
					}
					if( ficheroo )
					{	
						ficheroo = false;
						n = write( newsockfd, "desc", 5);				
						if ( n < 0 ) 
						{
							perror( "escritura en socket" );
							exit( 1 );
						}
						/** Funcion que realiza la descarga en modo UDP*/
						envio = descarga_archivo(6020,fichero);
						if( envio )
						{
							printf("Envio terminado de archivo...\n");
						}
						else
						{
							printf("Archivo no existe en al directorio...\n");
						}
						continue;	
					}
					/** Si no se recibe descarga se implementa el baash*/
					if( baash )
					{
						baash = false;
						getcwd(dirActual,sizeof(dirActual));//ACTUALIZAMOS RUTA ACTUAL
						printf("%s@%s:%s$ ",user,hostname,dirActual);//MOSTRAMOS PROMPT
						printf("\n");
						/** Copio en comando que ejecuta el cliente en buffer */
						strcpy(comando,buffer);
						/** Si la el cliente envia un "exit" finaliza la comunicacion con el cliente*/
						if( strcmp("exit\n",comando)==0 )	
						{
							printf("\nbaash: FINALIZACION DE EJECUCION DE BAASH\n");
							printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
							exit(0);
						}						
						if(!strcmp(comando,"\n"))  
						{		
							funcion_write( newsockfd, "exit", 5 );
							continue;
						}
						else									
						{
							leeComando(argV,comando);//GUARDAMOS LA CANTIDAD DE ARGUMENTOS Y GUARDAMOS EN (argv[0]) EL NOMBRE DEL COMANDO
							if(!strcmp(argV[0],"cd"))
							{
								chdir(argV[1]);//Cambiamos de directorio						
								funcion_write( newsockfd, "exit", 5 );
								continue;
							}
							/** BUSCAMOS ARCHIVO EN LOS DIRECTORIOS DEL PATH */
							buscaArchivo(argV[0], paths, executePath);
							if(executePath[0] == 'X')
							{
								printf("baash: NO SE ENCONTRO DIRECTORIO O ARCHIVO SOLICITADO\n");
								strcpy(salida,"baash: NO SE ENCONTRO DIRECTORIO O ARCHIVO SOLICITADO");
								funcion_write( newsockfd, salida, 2000 );
								funcion_write( newsockfd, "exit", 5 );
								memset( salida, '\0', 2000 );
								continue;
							}
							else
							{   /**Realizo el envio de la ejecuacion de comando al cliente*/
								FILE * pipe;
								char salida_aux[2000] = "";
								for( int pos = 1; pos < 20 ;pos++)
								{
									if(argV[pos] != NULL)
									{
										strcat(executePath, " ");
										strcat(executePath,argV[pos]);
									}
									else
									{
										break;
									}
								}
								//printf("comando: %s\n",executePath );
								pipe = popen(executePath,"r");
								if(pipe == NULL)
								{
									perror("No se pudo abrir pipe");
									exit(-1);
								}
								fgets(salida_aux , 2000, pipe);
								while(!feof(pipe))
								{	
									strcat(salida,salida_aux);
									fgets(salida_aux , 2000, pipe);
								}
								pclose(pipe);
								if(!strcmp(salida,""))
								{
									strcpy(salida," ");
								}
								//printf("%s\n",salida );
								n = write(newsockfd, salida, 2000);
								if(n < 0)
								{
									perror("Error al enviar el arvhivo:");
								}
								n = write(newsockfd, "exit", 5);
								if ( n < 0 ) 
								{
									perror( "escritura en socket" );
									exit( 1 ); 
								}
								memset(salida, 0, 2000);	
												
							}
						}
					}
				}
			}
		}
		else 
		{
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
		}
	}
return 0; 
} 
/**
 * @brief [Funcion que escribe en el socket]
 * 
 * @param newsockfd [descriptor de socket]
 * @param buffer [buffer de transmision]
 * @param size [longitud de buffer]
 */
void funcion_write(int newsockfd,char buffer[],int size)
{
	int n;
	n = write( newsockfd, buffer, size );
	if ( n < 0 ) 
	{
		perror( "escritura de socket" );
		exit( 1 );
	}
}
/**
 * @brief [Recibe los caracteres leidos del socket]
 * 
 * @param newsockfd [descriptor de socket]
 * @param buffer [buffer de recepcion]
 * @param size [lonfitud del buffer]
 */
void funcion_read(int newsockfd, char buffer[], int size)
{
	int n;
	n = read( newsockfd, buffer, TAM );
	if ( n < 0 ) 
	{
		perror( "lectura de socket" );
		exit( 1 );
	}
}
/**
 * @brief Muestra informacion del autor
 */
void MostrarInfoAutor()
{
	printf("\n///////////////////SISTEMA OPERATIVO II//////////////////////\n");		//
	printf("TRABAJO PRACTICO N°1: SOCKET-TCP-UDP \n");										//
	printf("ALUMNO: Salas Canalicchio, Sergio Enrique \n");							//
	//Cabecera();																	//
	printf("////////////////////////////////////////////////////////////\n");		//
}
/**
 * @brief Muestra informacion sobre fecha hora y nombre de la maquina
 */
void Cabecera()
{
	FILE *f; 
	char buffer[BUFFERSIZE+1];		
	char *match = NULL;				
	size_t bytesLeidos;				
	char dato1[BUFFERSIZE+1];		
	char dato2[BUFFERSIZE+1];		
	char dato3[BUFFERSIZE+1];		

	/** NOMBRE DEL EQUIPO */
	f = fopen("/proc/sys/kernel/hostname","r");
	if(f == NULL)
	{
		printf("NO SE HA PODIDIO ENCONTRAR EL ARCHIVO \n");
	}
	else
	{
		bytesLeidos = fread(buffer,1,sizeof(buffer),f);
		if(bytesLeidos == 0)
		{
			printf("FALLO LECTURA \n");
		}
		else
		{
			buffer[bytesLeidos] = '\0'; 
			sscanf(buffer, "%s",dato1);
			printf("NOMBRE DE LA MAQUINA: %s\n",dato1);
		}
	}

	fclose(f);
	/** FECHA Y HORA ACTUAL DEL SISTEMA */
	f = fopen("/proc/driver/rtc","r");
	if(f == NULL)
	{
		printf("NO SE HA PODIDIO ENCONTRAR EL ARCHIVO \n");
	}
	else
	{
		bytesLeidos = fread(buffer,1,sizeof(buffer),f);
		if(bytesLeidos == 0)
		{
			printf("FALLO LECTURA \n");
		}
		else
		{
			buffer[bytesLeidos] = '\0';
			match = strstr(buffer, "rtc_time");
			if(match==NULL)
			{
				printf("FALLO BUSQUEDA DE LINEA \n");
			}
			else
			{
				sscanf(match, "rtc_time	: %s",dato2);
			}
			match = strstr(buffer, "rtc_date");
			if(match==NULL)
			{
				printf("FALLO BUSQUEDA DE LINEA \n");
			}
			else
			{
				sscanf(match, "rtc_date	: %s",dato3);
			}
			printf("FECHA Y HORA ACTUAL: %s %s\n",dato3,dato2);
		}
	}
	fclose(f);
}
/**
 * @brief GUARDAMOS EN (ptahs[]) Y MOSTRAMOS LOS DIRECTORIOS DEL PATH 
 */
void buscaPaths(char *paths[])
{
	char *pathVar = getenv("PATH");// APUNTAMOS AL PRIMER DIRECTORIO DEL PATH (pathVar)
	paths[0] = strtok(pathVar, ":");// GUARDAMOS EN PRIMERA POSICION DEL ARREGLO DE PUNTEROS (paths[]) EL PRIMER DIRECTORIO DEL PATH
	/** MOSTRAMOS LISTA DE DIRECTORIOS DEL PATH */
	printf("\n/////////////LISTA DE DIRECTORIOS EN EL PATH////////////////\n");
	printf("PATH[%i] = %s \n",0,paths[0] );	
	for(int contadorPaths = 1; contadorPaths<BUFFERSIZE; contadorPaths++)//GUARDAMOS LOS SIGUINETES DIRECTORIOS DEL PATH (path[])
	{
		paths[contadorPaths] = strtok(NULL,":");
		if (paths[contadorPaths] == NULL)//SI NO SE ENCUENTRAN MAS DIRECTORIOS DEL PATH SALIMOS DEL CICLO FOR
		{
			break;
		}
		printf("PATH[%i] = %s \n",contadorPaths,paths[contadorPaths]);
	}
	strtok(NULL,":");
	printf("////////////////////////////////////////////////////////////\n");
}
/**
 * @brief [lee los comandos]
 * 
 * @param argv [argumento del comando]
 * @param cadena [comando completo]
 * 
 * @return [cantidad de argumentos]
 */
int leeComando(char *argv[],char *cadena)
{
	int argumentos = 0;							
	argv[0] = strtok(cadena, " \n");
	for(argumentos = 1; argumentos<BUFFERSIZE; argumentos++)
	{
		argv[argumentos] = strtok(NULL, " \n");		
		if (argv[argumentos] == NULL)			//SI NO SE ENCUENTRAN MAS ARGUMENTOS DEL COMANDO SALIMOS DEL CICLO FOR
		{
			break;
		}
	}
	return argumentos-1;						//RETORNAMOS Y NO CONTAMOS EL NOMBRE DEL COMANDO
}
/**
 * @brief [BUSCAMOS EL EJECUTABLE Y SE MUESTRA EN QUE CAMINO SE ENCUENTRA]
 * 
 * @param arch [parametro del argumento]
 * @param paths [camino]
 * @param execPath [guarda el camino]
 */
void buscaArchivo(char *arch, char *paths[], char *execPath)
{
	//BUSCAMOS EL EJECUTABLE Y SE MUESTRA EN QUE CAMINO SE ENCUENTRA
	char returnPath[50];
	int resultado;
	char searchDir[50] = "";
	char* archivo="";
	strcpy(returnPath, arch);
	
	/** PATH ABSOLUTO: */
	if(arch[0] == '/' || (arch[0] == '.' && arch[1] == '.' && arch[2] == '/'))
	{
		char *dir;
		char *nextDir;
		int pathCompleto = 0;		
		if(arch[0] == '/')
		searchDir[0] = '/';
		dir = strtok(arch,"/");
		nextDir = strtok(NULL,"/");		
		if(nextDir != NULL) //Verifica si es archivo
			strcat(searchDir,dir);
		else
		{
			nextDir = dir;
			pathCompleto = 1;
		}
		while((nextDir != NULL) && !pathCompleto)
		{
			dir = nextDir;
			nextDir = strtok(NULL,"/");
			strcat(searchDir,"/");
			if(nextDir != NULL)
				strcat(searchDir,dir);
		}
		archivo = dir;
	}
	/** PATH RELATIVO: */	
	else if(arch[0] == '.' && arch[1] == '/'){
		getcwd(searchDir, 50);
		strcat(searchDir,"/");
		archivo = strtok(arch, "/");
		archivo = strtok(NULL,"/");
	}	
	else{
		/** Busca en todos los directorios */
		int i;
		char aux[50];
		char aux2[50];
		char cadenaAux[256];
		char* m[1];
		m[0]= "";
		int bandera = 0;
		m[0] = getcwd(NULL,50);//camino o direccion actual		
		if(m[0] != NULL)//primero nos fijamos en el directorio actual
		{
			strcpy(aux2,m[0]);
			strcat(aux2,"/");//agregamos / en aux2
			strcat(aux2, arch);			
			resultado = access(aux2, F_OK);//Verifico si archivo existe en el path actual (camino o direccion actual)
			if(resultado==0)			   //si existe archivo guardo camino con archivo en execPath
			{
				strcpy(execPath, aux2);
				return;
			}
		}
		
		for(i = 0; i < 20; i++)// luego ejecutamos la busqueda en la secuencia PATH
		{
			strcpy(cadenaAux, arch);			
			if(paths[i] == NULL)
				break;
			
			while(1)
			{
				strcpy(aux,paths[i]);
				strcat(aux,"/");
				if(bandera == 0)
				{
					m[0] = strtok(cadenaAux, "/");
					if(m[0] == NULL){ break; }
					strcat(aux, m[0]);					
					resultado = access(aux, F_OK);
					if(resultado==0)
					{
						strcpy(execPath, aux);
						return;
					}
					strcat(aux,"/");
					bandera = 1;
				}
				else
				{
					m[0] = strtok(NULL, "/");
					if(m[0] == NULL){ break; }
					strcat(aux, m[0]);				
					resultado = access(aux, F_OK);
					if(resultado==0)
					{
						strcpy(execPath, aux);
						return;
					}
					strcat(aux,"/");
				}
				
			}
			bandera=0;
		}
		execPath[0] = 'X';		
		return;
	}	
	strcat(searchDir, archivo);
	resultado = access(searchDir, F_OK);//verifico en Path serchDir si existe archivo
	if(resultado==0)//si existe, paso
		strcpy(execPath, searchDir);//guardo path en execPath
	else
	{									
		execPath[0] = 'X';
	}									
}
/**
 * @brief [devuelve el parseo solicitado]
 * 
 * @param argv [es el texto a parsear]
 * @param arroba [es el caracter hasta donde se quiere parsear]
 * @return [retorna el texto que se requiera]
 */
char* devuelve_parseo(char argv[], const char arroba[])
{
  char *token;
  token = strtok( argv, arroba );
  return token;
}
/**
 * @brief [Envia archivo al cliente en modo de no conexion udp]
 * 
 * @param puert0 [puerto del servidor]
 * @param buffer [nombre del fichero]
 * @return [retorna 1 si se envio correctamente el archivo o 0 si no se encuentra en el directorio]
 */
int descarga_archivo(int puerto, char buffer[])
{	/** VARIABLES UTILIZAS PARA NO CONECION */
	int sockfd_udp;
	int puerto_udp;
	socklen_t tamano_direccion_udp;
	char buffer_aux[ TAM ];
	struct sockaddr_in serv_addr_udp;
	int n_udp;
	char tamanio[100] = {0};	

	sockfd_udp = socket( AF_INET, SOCK_DGRAM, 0 );
	if (sockfd_udp < 0) 
	{ 
		perror("ERROR en apertura de socket");
		exit( 1 );
	}
	/** Limpieza de la estructura */
	memset( &serv_addr_udp, 0, sizeof(serv_addr_udp) );
	puerto_udp = 6020;
	/** Carga de la familia de direcciones */
	serv_addr_udp.sin_family = AF_INET;
	serv_addr_udp.sin_addr.s_addr = INADDR_ANY;
	/** Carga el numero de puerto_udp :primero los bit mas significativos big-endian */
	serv_addr_udp.sin_port = htons( puerto_udp );
	memset( &(serv_addr_udp.sin_zero), '\0', 8 );
	if( bind( sockfd_udp, (struct sockaddr *) &serv_addr_udp, sizeof(serv_addr_udp) ) < 0 ) 
	{
		perror( "ERROR en binding" );
		exit( 1 );
	}	
    printf( "\nSocket disponible en UDP: %d\n", ntohs(serv_addr_udp.sin_port) );
	tamano_direccion_udp = sizeof( struct sockaddr_in/*sockaddr*/ );
	memset( buffer_aux, 0, TAM );
	n_udp = recvfrom( sockfd_udp, buffer_aux, TAM-1, 0, (struct sockaddr *)&serv_addr_udp, &tamano_direccion_udp );
	if ( n_udp < 0 ) 
	{
		perror( "lectura de socket" );
		exit( 1 );
	}
	printf("%s\n",buffer_aux );
	/** Verifico si existe el fichero */
	FILE *fp = fopen(buffer, "rb");
	    if (fp == NULL) 
	    {
	       n_udp = sendto( sockfd_udp, (void *)"fin", 5, 0, (struct sockaddr *)&serv_addr_udp, tamano_direccion_udp  );
			if ( n_udp < 0 ) {
				perror( "Escritura en socket" );
				exit( 1 );
			}
			
			close(sockfd_udp);
	        return 0;
	    }
	char *filename =basename(buffer); 
    if (filename == NULL)
    {
        perror("Can't get filename");
        exit(1);
    }
    char buff[60] = {0};
    strncpy(buff, filename, strlen(filename));
    printf("Nombre de fichero: %s\n",buff );
    /** Envio nombre de fichero */
    if (sendto( sockfd_udp, buff, 60, 0, (struct sockaddr *)&serv_addr_udp, tamano_direccion_udp ) == -1)
    {
        perror("Can't send filename");
        exit(1); 
    }   
    int len;
	FILE *f_p = fopen(buffer, "rb");
    fseek(f_p, 0, SEEK_END);
    len = ftell(f_p);
    sprintf(tamanio, "%d", len);
    printf("Tamaño de fichero: %s bytes\n",tamanio );
     /** Envio tamaño de fichero*/
    if (sendto( sockfd_udp, (void *)tamanio, 60, 0, (struct sockaddr *)&serv_addr_udp, tamano_direccion_udp ) == -1)
    {
        perror("Can't send filename");
        exit(1);
    }
    fclose(f_p);
	int nn; 
    char sendline[4096] = {0}; 
	int capacidadd = 0;
	/** Envio contenido de fichero */
    while (!feof(fp)/*(nn = fread(sendline, sizeof(char), 4096, fp)) > 0*/) 
    {    	
    	nn = fread(sendline, sizeof(char), 4096, fp);
    	capacidadd = capacidadd + nn;
        if (nn != 4096 && ferror(fp)) 
        {
            perror("Read File Error");
            exit(1);
        }        
        if (sendto( sockfd_udp, (void*)sendline, nn, 0, (struct sockaddr *)&serv_addr_udp, tamano_direccion_udp ) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
	    //int delay = 1000000;
	    int delay = 100000;
	    while(delay > 0)
	    {
	     	delay = delay - 1;
	    }
        memset(sendline, 0, 4096);
    }
    /**Envio exit para  hacer informarle al cliente que termino de enviar*/
    if (sendto( sockfd_udp, (void*)"exit", 5, 0, (struct sockaddr *)&serv_addr_udp, tamano_direccion_udp ) == -1)
    {
        perror("Can't send file");
        exit(1);
    }
    printf("Tamaño de fichero enviado: %d bytes\n",capacidadd );
    printf("Se envio el archivo...\n" );
    fclose(fp);
 	close(sockfd_udp);
	return 1;
}
