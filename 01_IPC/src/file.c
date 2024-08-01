#include "../include/deliverymanager.h"
#define puerto_files 8020
#define IMAGES_PATH "/your/directory/log.zip"

int32_t sockfd, sock_cli;
struct sockaddr_in serv_addr;
struct sockaddr_in client_addr;
uint32_t client_len; // tamaño de la dirección del cliente
char buffer[TAM];
ssize_t n;	   // hubo que declarar n como ssize_t para que no pierda información al usarse en send() y recv()
long *logsize; // para guardar size de log
char *logmd5;  // para guardar md5 de log
int32_t main()
{

	configurar_socket(); // configura y abre el socket para transmitir

	escuchando(); // espera que se conecte un socket

	// En el servidor (file)
	printf("Esperando conexiones en el puerto %d\n", puerto_files);

	while (1)
	{
		conectar_cliente();
		/*
			FILE*  file= fopen(IMAGES_PATH, "r");
			if ( file != NULL ) {
				int32_t fd=fileno(file);
				struct stat file_stat;
				fstat(fd, &file_stat);
				if ((sendfile(sockfd, fd, 0,(size_t) file_stat.st_size)) < 0) {
					perror("error al transferir archivo");
					exit(1);
				}
				fclose(file);
				//una vez terminada la transferencia se envia EOF para indicar la finalizacion
				strcpy(buffer,"EOF");
				send(sockfd,buffer,sizeof(buffer),0);
				printf("Terminando descarga\n");
			}
			else {
				perror("archivo inexistente\n");
			}

		*/

		// char *mensaje = "conexión";

		n = send(sock_cli, "conexion", strlen("conexion"), 0);
		// n = send( sock_cli, mensaje, strlen(mensaje),0 );
		if (n < 0)
		{
			perror("fallo en enviar info");
		}

		 logsize=calloc(1,sizeof(long));
		 logmd5=calloc(TAM,sizeof(char));

		 read_log();

		 char * logsizeChar=calloc(TAM,sizeof(char)); 
		 sprintf(logsizeChar,"%ld",*logsize);
		 printf("tamanio %s\n",logsizeChar);
		 printf("hash md5: %s\n",logmd5);

		// envía tamaño
		 n = send(sock_cli, logsizeChar , strlen(logsizeChar), 0);
		if (n < 0)
		{
			perror("fallo en enviar info");
		}
		// espera confirmaciòn
	    n = recv(sock_cli, buffer, TAM - 1, 0);
		if (n < 0)
		{
			perror("lectura de socket");
			exit(1);
		}

		// envía  hash md5
		 n = send(sock_cli, logmd5 , strlen(logmd5), 0);
		if (n < 0)
		{
			perror("fallo en enviar info");
		}

		// espera confirmaciòn
	    n = recv(sock_cli, buffer, TAM - 1, 0);
		if (n < 0)
		{
			perror("lectura de socket");
			exit(1);
		}

		// enviar el archivo
		enviar_log();

		 free(logsize);
		 free(logmd5);
		 free(logsizeChar);

		// exit(0);
	}
	return 0;
}

/*
	Configura el socket con el que se enviará la imagen
*/
void configurar_socket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons((uint16_t)puerto_files);
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("conexion");
		exit(1);
	}
}

/*
	Abre conexión para que se conecte un clietne
*/
void escuchando()
{
	listen(sockfd, 1);
	client_len = sizeof(client_addr);
}

/*
	Busca el archivo solicitado, avisa si lo encontró o no, y en caso afirmativo
	envía dicho archivo
*/
void read_log()
{

	char img[TAM];
	long size = 0;

	sprintf(img, "%s", IMAGES_PATH); // Path de la imagen

	FILE *imgn;
	imgn = fopen(img, "r");
	if (imgn == NULL)
	{
		perror("file");
		return;
	}

	// Calcula el tamaño de la imagen
	fseek(imgn, 0, SEEK_END);
	size = ftell(imgn);
	fclose(imgn);

	// guarda tamaño para enviar
	// char size_s[TAM] = "";
	logsize = malloc(sizeof(long));
    if (logsize == NULL) {
        perror("error al reservar memoria para logsize");
        //exit(EXIT_FAILURE);
    }
	*logsize = size;

	// char *md5s = md5(img, 0);
	logmd5 = md5(img, 0);

	// if(md5s != NULL){}
	//if (logmd5 != NULL)
	//{
	//}

	// guarda en buffer el tamaño y hash para enviar
	// sprintf(buffer, "Download %s %s", size_s, md5s);

	// Envía la imagen por socket
	// enviar_imagen(img, &size);
}

/*
	Conecta con el cliente que usó su dirección
*/
void conectar_cliente()
{

	sock_cli = accept(sockfd, NULL, NULL);
	if (sock_cli < 0)
	{

		perror("accept");
		exit(1);
	}
	printf("conectado\n");
}

void enviar_log() {
    char img[TAM];

    sprintf(img, "%s", IMAGES_PATH); // Path de la imagen
    FILE *imgn = fopen(img, "r");
    if (imgn == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    // Obtener el tamaño del archivo
    fseek(imgn, 0, SEEK_END);
    //long size = ftell(imgn);
    fseek(imgn, 0, SEEK_SET);

    // Leer y enviar el archivo en fragmentos
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, TAM, imgn)) > 0) {
        if (send(sock_cli, buffer, bytes_read, 0) == -1) {
            perror("Error al enviar el archivo");
            exit(1);
        }
    }

    fclose(imgn);
}
/*
void enviar_log(){
	int32_t imgfd;
	char img[TAM];
	long size = 0;

	sprintf(img, "%s", IMAGES_PATH); // Path de la imagen
//	imgfd = open(img, O_RDONLY);
//	if(imgfd == -1)
//  		{
//    		perror("error");
//    		exit(EXIT_FAILURE);
//  		}
//	printf("%s Enviando log %s\n",KBLU,KNRM);

	FILE *imgn = fopen(img, "r");
    if (imgn == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

//	size_t to_send = (size_t) size;
//  	ssize_t sent;
//	off_t offset = 0;

	// Obtener el tamaño del archivo
    fseek(imgn, 0, SEEK_END);
    size = ftell(imgn);
    fseek(imgn, 0, SEEK_SET);

//	while(((sent = sendfile(sock_cli, imgfd, &offset, to_send)) > 0)
//			&& (to_send > 0))
//	{
//		to_send -= (size_t) sent;
//		printf(" %sSe envió %lu %s\n", KGRN, sent, KNRM);
//	}

//	if (sent == 0 && to_send == 0) {
  //  printf("Se ha enviado todo el archivo correctamente\n");
//} else {
  //  perror("Error al enviar el archivo");
    //exit(1);/
//}

//	printf(" %sSe envió %lu %s\n", KGRN, size, KNRM);

//	close(imgfd);

	// Leer y enviar el archivo en fragmentos
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, TAM, imgn)) > 0) {
        if (send(sock_cli, buffer, bytes_read, 0) == -1) {
            perror("Error al enviar el archivo");
            exit(1);
        }
    }

    fclose(imgn);
}
*/
/*
		FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("/home/diego/Escritorio/Sistemas-Operativos-2/TP1/soii-2021-ipc-adgko/archivos/logs/productor_1.log", "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("Retrieved line of length %zu:\n", read);
		printf("%s", line);
	}

	fclose(fp);
	if (line)
		free(line);
*/
// read_log();

// n = recv( sock_cli, buffer, TAM-1, MSG_WAITALL );
//			if ( n < 0 ) {
//		 	 perror( "lectura de socket" );
//			  exit(1);
//			}

//	printf( "%sRecibí: %s%s\n", KBLU,buffer,KNRM );
//    n = send( sock_cli, "12341234 respuesta", strlen("12341234 respuesta"),0 );
//									if(n < 0){
//										perror("fallo en enviar info");
//									}

//	n = recv(sock_cli, buffer, TAM - 1, MSG_WAITALL);
//   if (n < 0) {
//       perror("lectura de socket");
//       exit(1);
//   }
// if(n == 0){printf("Se cerró la comunicación\n");}
// A ver si llegó
/*n = recv( sock_cli, buffer, TAM-1, MSG_WAITALL );
			if ( n < 0 ) {
			  perror( "lectura de socket" );
			  exit(1);
			}


			printf( "%sRecibí: %s%s\n", KBLU,buffer,KNRM );
memset( buffer, 0, TAM );
*/

/*	FILE *imgn;
	imgn = fopen(IMAGES_PATH, "rb");
	if(imgn == NULL)
	{
		perror("file");
	}

	char buffer[TAM];
		size_t bytesRead;
		while ((bytesRead = fread(buffer, 1, sizeof(buffer), imgn)) > 0) {
			if (send(sock_cli, buffer, bytesRead, 0) == -1) {
				perror("Error al enviar datos al cliente");
				break;
			}
		}

		// Enviar indicador de fin de archivo
		send(sock_cli, "EOF", 3, 0);

		// Cerrar el archivo y conexión con el cliente
		fclose(imgn);
*/
// n = send(sock_cli,logsize,sizeof(long),0);
// if(n<0) {
//	perror("fallo en enviar info");
//	}

// en caso de exito se abre el archivo y se inicia transferencia por medio de sendFile

/*
	FILE*  file= fopen(IMAGES_PATH, "r");
	if ( file != NULL ) {
		int32_t fd=fileno(file);
		struct stat file_stat;
		fstat(fd, &file_stat);
		if ((sendfile(sock_cli, fd, 0,(size_t) file_stat.st_size)) < 0) {
			perror("error al transferir archivo");
			exit(1);
		}
		fclose(file);
		//una vez terminada la transferencia se envia EOF para indicar la finalizacion
		strcpy(buffer,"EOF");
		send(sock_cli,buffer,sizeof(buffer),0);
		printf("Terminando descarga\n");
*/
