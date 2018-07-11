#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define max_buffer_size  1024*128
 
 
/*program pouziva 2 vlakna 
	- jedno na citanie - reader_thread 
	- druhe na vypisovanie - writer_thread 
  pri synchronizacii vlakien budem potrebovat dve pthread_cond_t a jeden pthread_mutex_t 		
*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t reader_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t writer_cond = PTHREAD_COND_INITIALIZER;
//pomocna globalna premenna 
int chars_read = 0;
 
// pre zmenu velkosti buffera staci zmenit konstantu max_buffer_size vyssie
char buffer[max_buffer_size];


/*thread_read 
	nacita maximalne max_buffer_size znakov do globalnej premennej buffer
	na nacitavanie do buffera sa pouzije read, ktorej argumenty budu 
		0 - pre nacitavanie zo standardneho vstupu,
		buffer - pole znakov,
		max_buffer_size - kolko znakov sa ma nacitat
	read vrati
		< 0	- ak nastala chyba
		inak pocet znakov, ktore sa nacitali
*/
void *thread_read(void *data){
	int read_chars = 1;
	while(read_chars){
		// kriticka sekcia kodu - potrebujeme pouzit mutex
		pthread_mutex_lock(&mutex);

		read_chars = read(0, buffer, max_buffer_size);
		chars_read = read_chars;

		pthread_cond_signal(&reader_cond);
		if (read_chars)
			pthread_cond_wait(&writer_cond, &mutex);

		pthread_mutex_unlock(&mutex);
		//koniec kritickej sekcie	
	}	
    pthread_exit(NULL);
	
} 


/*thread_write 
	kriticka sekcia kodu spociva vo vypisovani znakov z buffera
	na vypisovanie sa pouzije write s argumentmi
		1 - pre standarndy vystup,
		buffer - pole znakov,
		chars_read - tolko znakov, kolko bolo nacitanych pri poslednom nacitavani do buffera
	write vrati 
		< 0 - chyba pri vypise
		= 0 - nic sa nevypisalo
		> 0 pocet vypisanych znakov
*/
void *thread_write(void *data){
	int write_result = 1;
	while(write_result){
		//kriticka sekcia kodu - potrebujeme pouzit mutex
		pthread_mutex_lock(&mutex);

		write_result = write(1, buffer, chars_read);
		chars_read = 0;

		pthread_cond_signal(&writer_cond);
		if (write_result)
			pthread_cond_wait(&reader_cond, &mutex);
			
		pthread_mutex_unlock(&mutex);
		//koniec kritickej sekcie
	}
	pthread_exit(NULL);
}
 
int main (){
	
	//vytvorim dve vlakna - reader_thread a writer_thread - kontroly som pre istotu vynechal a predpokladam, ze vytvorenie zbehne bez problemov
	pthread_t reader_thread, writer_thread;
    pthread_create(&reader_thread, NULL, thread_read, NULL);
    pthread_create(&writer_thread, NULL, thread_write, NULL);
    
	//pockame, kym dobehnu obe vlakna a potom skoncime
	pthread_join(reader_thread, NULL);
	pthread_join(writer_thread, NULL);
	return 0;

}
