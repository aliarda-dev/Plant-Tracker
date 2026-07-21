#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht.h"

#include "esp_wifi.h" //Wifi surucusu
#include "nvs_flash.h" //Wifi surucusunun bazi bilgileri burada tutulur
#include "esp_event.h" //Wifi baglantısı saglandı mi gibi kontrol eden fonksiyonlara sahip
#include "esp_log.h"
#include "esp_netif.h" //Wifinin TCP/IP katmani hazirlanmasi

#include "esp_http_client.h" //htpp istegi icin (telegram kullanicaz)
#include "esp_crt_bundle.h"

#include "esp_adc/adc_oneshot.h" //Toprak nem sensoru icin

#include "esp_timer.h"









#define WIFI_SSID "xxxx" //wifi adi
#define WIFI_PASS "xxxx" //wifi sifre 

#define BOT_TOKEN "xxx" //telegram bot token
#define CHAT_ID "xxx" //telegram bot chat id

#define MAVI_LED GPIO_NUM_5
#define YESIL_LED GPIO_NUM_6
#define KIRMIZI_LED GPIO_NUM_7

#define TOPRAK_PIN ADC_CHANNEL_0
adc_oneshot_unit_handle_t adc_handle;


#define DHT22_PIN GPIO_NUM_8

static const char *TAG = "WiFi"; //Loglarda gorunecek isim
static void wifi_event_handler( //Espnin olay kontrol dongusunu kontrol eden fonksiyon
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data);
void wifi_init(void);        

bool DhtDegerler (float*,float*);

void telegram_message(const char *);

void toprakPin_init(void);
void toprakDeger(int*);



void app_main(void){

    gpio_set_direction(MAVI_LED,GPIO_MODE_OUTPUT);
    gpio_set_direction(YESIL_LED,GPIO_MODE_OUTPUT);
    gpio_set_direction(KIRMIZI_LED,GPIO_MODE_OUTPUT);



    wifi_init();
    toprakPin_init();

    vTaskDelay(pdMS_TO_TICKS(8000));


    float sicaklik=0;
    float nem=0;

    int toprakDegeri=0;
    int esikDegeri=4000;

    
    int64_t son_mesaj_zaman_sicaklik=0;
    int64_t son_mesaj_zaman_sulama=0;
    int64_t son_mesaj_zaman_havaKosul=0;

    const int64_t mesaj_aralik=10*60*1000000; //10 dakika esp timera baglı bu hesaplama

     gpio_set_level(MAVI_LED,1);
     gpio_set_level(YESIL_LED,1);
     gpio_set_level(KIRMIZI_LED,1);


            

    while(1){

        bool sensor_kontrol=DhtDegerler(&sicaklik,&nem);

        vTaskDelay(pdMS_TO_TICKS(5000)); 

         if(sensor_kontrol==false){
            printf("DHT okunamadi hata!!\n\n");
            continue;
        }

        int64_t simdi=esp_timer_get_time();

        if(gpio_get_level(KIRMIZI_LED)==0){
            gpio_set_level(KIRMIZI_LED,1);
        }

        if(simdi-son_mesaj_zaman_sicaklik>mesaj_aralik){ //istedigimiz aralikta odanin kosullarini telegrama gonderecegiz

            char mesaj[50];

            snprintf(mesaj,sizeof(mesaj),"Sicaklik %.2f'C | Nem %.2f/100",sicaklik,nem);

            telegram_message(mesaj);

            son_mesaj_zaman_sicaklik=simdi;
        }





        toprakDeger(&toprakDegeri);
        vTaskDelay(pdMS_TO_TICKS(1000));

        if(toprakDegeri<esikDegeri && simdi-son_mesaj_zaman_sulama>mesaj_aralik/2){ //5 dakika
            telegram_message("Bitkinizi acil sulayin!!!");
            gpio_set_level(YESIL_LED,0);
            vTaskDelay(pdMS_TO_TICKS(500));
            gpio_set_level(YESIL_LED,1);

            son_mesaj_zaman_sulama=simdi;
        }

        if((sicaklik>40 || nem>80 || nem<30) && simdi-son_mesaj_zaman_havaKosul>mesaj_aralik/2){ //5 dakika
            telegram_message("Bitkiniz icin gerekli hava kosullari saglanmiyor!");
            gpio_set_level(KIRMIZI_LED,0);
            son_mesaj_zaman_havaKosul=simdi;
        }


        vTaskDelay(pdMS_TO_TICKS(5000));


        
    }

    



}






bool DhtDegerler (float* sicaklik,float* nem){

    esp_err_t sonuc = dht_read_float_data(DHT_TYPE_AM2301,DHT22_PIN,nem,sicaklik);

    if(sonuc==ESP_OK){
        printf("Sicaklik: %.2f\n",*sicaklik);
        printf("Nem: %.2f",*nem);
        vTaskDelay(pdMS_TO_TICKS(3500));
        return true;
    }

    else{
        printf("DHT duzgun calismiyor!!\n");
        return false;
    }

    

}



static void wifi_event_handler( //Espnin olay kontrol dongusunu kontrol eden fonksiyon
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data){

            if(event_base==WIFI_EVENT && event_id==WIFI_EVENT_STA_START){ //Wifi surucusu basladiysa

                ESP_LOGI(TAG,"Wi-fi Basladi!\n\n");

                esp_wifi_connect();

            } 

            else if(event_base==WIFI_EVENT && event_id==WIFI_EVENT_STA_DISCONNECTED){

                ESP_LOGI(TAG,"Baglanti Koptu Tekrar Deneniyor...\n\n");

                esp_wifi_connect();
            }

            else if(event_base==IP_EVENT && event_id==IP_EVENT_STA_GOT_IP){ //IP alındıgında

                ESP_LOGI(TAG,"Wi-fi baglandi ve ip alindi...\n\n");
            }

        }



void wifi_init(void){

        nvs_flash_init(); //Flash bellek hazirlanmasi

        esp_netif_init(); //TCP_IP katmanini baslatilmasi

        esp_event_loop_create_default(); //Event sisteminin olusturulmasi

        esp_netif_create_default_wifi_sta(); //Station arayuzunun olusmasi


        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //Wifi driver ayarlari

        esp_wifi_init(&cfg); //Driveri baslatma


        esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,NULL); //Wifi Event Dinleme (Wifi event handler fonksiyonunu kullandıgımız yer)

        esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&wifi_event_handler,NULL,NULL); //IP Event Dinleme


        wifi_config_t wifi_config = { //Wifi Bilgileri
            .sta={
                .ssid=WIFI_SSID,
                .password=WIFI_PASS,
            },
        };

        esp_wifi_set_mode(WIFI_MODE_STA); //Station modu gecis

        esp_wifi_set_config(WIFI_IF_STA,&wifi_config); //Ayarları yukleme

        esp_wifi_start(); //Wi-fi surucusunu baslatma

}        


void telegram_message(const char *message){

    char url[512];

    snprintf(url,sizeof(url),"https://api.telegram.org/bot%s/sendMessage",BOT_TOKEN);



    char mesajiniz[512];

    snprintf(mesajiniz,sizeof(mesajiniz),"chat_id=%s&text=%s",CHAT_ID,message);

    esp_http_client_config_t config={.url=url,.crt_bundle_attach=esp_crt_bundle_attach,};

    esp_http_client_handle_t client=esp_http_client_init(&config);

    
    if(client==NULL) printf("Client olusturulamadi!\n");

    esp_http_client_set_method(client,HTTP_METHOD_POST);

    esp_http_client_set_header(client,"Content-Type","application/x-www-form-urlencoded");

    esp_http_client_set_post_field(client,mesajiniz,strlen(mesajiniz));

    esp_err_t err=esp_http_client_perform(client);


    if(err==ESP_OK) printf("Mesaj gonderildi..\n");

    else printf("Hata %s\n",esp_err_to_name(err));


    esp_http_client_cleanup(client);
}


void toprakPin_init(void){

    adc_oneshot_unit_init_cfg_t init_config={.unit_id=ADC_UNIT_1};

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config,&adc_handle));

    adc_oneshot_chan_cfg_t config={.bitwidth=ADC_BITWIDTH_DEFAULT,.atten=ADC_ATTEN_DB_12};

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle,TOPRAK_PIN,&config));
}


void toprakDeger(int* deger){

    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle,TOPRAK_PIN,deger));

    printf("Toprak ADC: %d\n",*deger);
}








