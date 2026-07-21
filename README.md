***Plant Tracker***

DHT22 ve FC28 sensörlerini entegre çalıştırarak kodumda yazdığım koşullar sağlandığında telegram botuna mesaj yollayan bir IoT projesi. İlk IoT proje deneyimim oldu zorlandığım taraf ESP32'nin wifiye bağlaması oldu, buna gerçekten çok vakit ayırdım.

***Uğraştığım Hatalar***

* C make list mantığını biraz geç öğrendim bu yüzden compiler sürekli hata veriyordu
* Https sertifika hataları
* Main task overflow hatası, menucfonig defaultu 3584 bytedi ben bunu 8192 yaptim (https istekleri buna neden oldu)
* DHT22 Phase B hatası aldım. Çözümü delay kullanmak oldu wifi istekleri ile aynı anda kullanılınca sıkıntı çıkarabiliyor.
* RGB led 5V bağlı iken yanmıyordu. Sebebi ESP32'nin 5V çıkışının aktif olmamasıydı. Redditte dolaşmaktan projeden bıkmaya başlamıştım sonrasında kart üstündeki N-OUT köprüsünü öğrendim 5V güç çıkışı için bunun lehimlenmesi gerekiyormuş onu lehimledim.
* ve daha niceleri


***Donanım***

* ESP32S3
* DHT22 (Sıcaklık ve Nem sensörü)
* FC-28 (Toprak Nem Sensörü)
* Ortak Anot RGB Led
* 4x330R Direnç (Üç tanesi rgb led için geriye kalan bir tanesi de dht sensoru için)


***Nasıl Çalışır?***

* DHT22 sensöründen sıcaklık ve nem değerleri okunur ve gerekli variablelara aktarılır
* FC28 sensöründen nem değeri okunarak gerekli variablea aktarılır
* Wifi bağlantısı kurulur
* Değerler lcd ekranda gösterilir
* Belirlenen aralıkta telegram botuna bitkinin içinde bulunduğu odanın sıcaklık ve nem değerleri gönderilir
* Toprak nem değeri eşik değeri altına düşerse bota uyarı mesajı yollanır
* Sıcaklık ve nem değerleri belirlenen değerler dışına saparsa bota başka bir uyarı mesajı yollanır
* Zamanlama kontrolu olarak esp_timer_get_time() kullanılır

***Teknik Notlar***

* Wifi bağlantısı ve Telegram API kısmında AI'dan destek aldım. İlk IoT projem olduğu için ESP'nin ağ yapısını tek başıma kurmakta zorlandım. Kodu tamamladıktan sonra birkaç gün boyunca kullandığım her fonksiyonu ve event yapısını tek tek inceleyerek nasıl çalıştığını öğrenmeye çalıştım.
* Yine aynı şekilde kodun belirli kısımlarında ai desteği aldım.

***Geliştirme Fikirleri***

* ESP32 hafızası üzerinde dosya açılarak hataların geldiği saatler data olarak tutulabilir.
  

🎥 Proje Demo



📷 Proje PCB Medyası
