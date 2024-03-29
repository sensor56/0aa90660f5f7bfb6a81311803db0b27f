// Supports PDF d'Ateliers Arduino par www.mon-club-elec.fr
// http://www.mon-club-elec.fr/pmwiki_mon_club_elec/pmwiki.php?n=MAIN.ATELIERS
// par X. HINAULT - tous droits réservés - 2013 - GPLv3

// code :20. Serveur Arduino : Contrôler un servomoteur par envoi d'une requête Ajax avec paramètre numérique sur un clic souris dans un canvas utilisé en « slider »

// --- Inclusion des librairies ---

#include <Utils.h> // inclusion de la librairie 
#include <Servo.h> // inclut la librairie Servo
#include <SPI.h> // librairie SPI - obligatoire avec librairie Ethernet
#include <Ethernet.h> // librairie Ethernet

// --- Déclaration des variables globales ---

//--- l'adresse mac = identifiant unique du shield
// à fixer arbitrairement ou en utilisant l'adresse imprimée sur l'étiquette du shield
byte mac[] = {  0x90, 0xA2, 0xDA, 0x00, 0x1A, 0x71 };

//----- l'adresse IP fixe à utiliser pour le shield Ethernet --- 
IPAddress ipLocal(192,168,1,100); // l'adresse IP locale du shield Ethernet
// ATTENTION : il faut utiliser une adresse hors de la plage d'adresses du routeur DHCP
// pour connaitre la plage d'adresse du routeur : s'y connecter depuis un navigateur à l'adresse xxx.xxx.xxx.1
// par exemple : sur livebox : plage adresses DHCP entre .10 et .50 => on peut utiliser .100 pour le shield ethernet

// --- Déclaration des objets utiles pour les fonctionnalités utilisées ---

Servo servo; // déclaration d'un objet servomoteur
const int brocheServo=2; // broche du servomoteur 

//--- création de l'objet serveur ----
EthernetServer serveurHTTP(80); // crée un objet serveur utilisant le port 80 = port HTTP

Utils utils; // déclare objet racine d'accès aux fonctions de la librairie Utils
long params[6]; // déclare un tableau de long - taille en fonction nombre max paramètres attendus

String chaineRecue=""; // déclare un string vide global pour réception chaine requete
String chaineAnalyse=""; // string vide global pour cahine retenue pour analyse
int comptChar=0; // variable de comptage des caractères reçus 
//----- constantes de paramétrage du servomoteur ---- 
const int posMin=550; // largeur impulsion en µs correspondant à la position 0° du servomoteur
const int posMax=2350; // largeur impulsion en µs correspondant à la position 180° du servomoteur
//----- valeur pour un Futaba S3003 - à adapter à votre situation

void setup()   { // debut de la fonction setup()

// --- ici instructions à exécuter 1 seule fois au démarrage du programme --- 

// ------- Initialisation fonctionnalités utilisées -------  

Serial.begin(115200); // Initialise connexion Série 

servo.attach(brocheServo, posMin, posMax); // attache le servomoteur à la broche 
   // et initialisation des positions extremes
   
//---- initialise la connexion Ethernet avec l'adresse MAC du module Ethernet, l'adresse IP Locale 
//----  +/- l'adresse IP du serveurDNS , l'adresse IP de la passerelle internet et le masque du réseau local

//Ethernet.begin(mac); // forme pour attribution automatique DHCP - utilise plus de mémoire Flash (env + 6Ko)
Ethernet.begin(mac, ipLocal); // forme conseillée pour fixer IP fixe locale
//Ethernet.begin(mac, ipLocal, serverDNS, passerelle, masque); // forme complète

delay(1000); // donne le temps à la carte Ethernet de s'initialiser

Serial.print(F("Shield Ethernet OK : L'adresse IP du shield Ethernet est : " )); 

Serial.println(Ethernet.localIP()); 

//---- initialise le serveur ----
serveurHTTP.begin(); 
Serial.println(F("Serveur Ethernet OK : Ecoute sur port 80 (http)")); 

} // fin de la fonction setup()

void loop(){ // debut de la fonction loop()

// crée un objet client basé sur le client connecté au serveur
  EthernetClient client = serveurHTTP.available(); 
  
  if (client) { // si l'objet client n'est pas vide
   // le test est VRAI si le client existe 
  
   
    // message d'accueil dans le Terminal Série
      Serial.println (F("--------------------------")); 
      Serial.println (F("Client present !")); 
      Serial.println (F("Voici la requete du client:")); 

  ////////////////// Réception de la chaine de la requete //////////////////////////
  
      //-- initialisation des variables utilisées pour l'échange serveur/client
      chaineRecue=""; // vide le String de reception
      comptChar=0; // compteur de caractères en réception à 0  

    if (client.connected()) { // si le client est connecté

      /////////////////// Réception de la chaine par le réseau ////////////////////////      
      while (client.available()) { // tant que  des octets sont disponibles en lecture
      // le test est vrai si il y a au moins 1 octet disponible
       
        char c = client.read(); // l'octet suivant reçu du client est mis dans la variable c
        comptChar=comptChar+1; // incrémente le compteur de caractère reçus

        Serial.print(c); // affiche le caractère reçu dans le Terminal Série

        //--- on ne mémorise que les n premiers caractères de la requete reçue
        //--- afin de ne pas surcharger la RAM et car cela suffit pour l'analyse de la requete
        if (comptChar<=100) chaineRecue=chaineRecue+c; // ajoute le caractère reçu au String pour les N premiers caractères
        //else break; // une fois le nombre de caractères dépassés sort du while 
           
      } // --- fin while client.available = fin "tant que octet en lecture"
      
    Serial.println (F("Reception requete terminee"));

     //------ analyse si la chaine reçue est une requete GET avec chaine format /&chaine= --------
    if (chaineRecue.startsWith("GET /&")) {

          //----- extraction de la chaine allant de & à =
          int indexStart=chaineRecue.indexOf("&"); 
          int indexEnd=chaineRecue.indexOf("="); 
          Serial.print (F("index debut =")); 
          Serial.println (indexStart);     
          Serial.print (F("index fin =")); 
          Serial.println (indexEnd);     
    
          chaineAnalyse=chaineRecue.substring(indexStart+1,indexEnd); // garde chaine fonction(xxxx) à partir de GET /&fonction(xxxx)=
          // substring : 1er caractère inclusif (d'où le +1) , dernier exclusif 

             // -- message debug -- 
             Serial.print (F("Chaine recue = "));
             Serial.println (chaineAnalyse);
          
             // -- analyse de la chaine à analyser -- 
             
             if (chaineAnalyse!="") { // si une chaine à analyser non vide 

                //------ si la chaine servo(xxx) est reconnue 
                if(utils.testInstructionLong(chaineAnalyse,"servo(",1,params)==true) { // si chaine FONCTION(valeur) bien reçue 

                    Serial.println("Arduino a recu le parametre : " + (String)params[0]);
                                    
                   // action à exécuter
                   servo.write(params[0]); // positionne le servomoteur dans l'angle voulu 
      
                   // envoi reponse à la requete Ajax
                   envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client
                   client.print(F("Chaine recue :")); 
                   client.println(chaineAnalyse); 
                   client.print(F("Parametre recu :")); 
                   client.println(params[0]); 
  	  //-- une fois la réponse Ajax terminée, la fonction de callback drawData est exécutée     
                } // fin si testInstructionLong==true
             
             } // fin // si une chaine analyse a été reçue                        
                  
    } // fin if GET /&    

                // +/- message si chaine pas reconnue         
                else { // sinon si chaine pas reconnue 
                   
                   // envoi reponse à la requete Ajax
                   envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client
                   client.print(F("Chaine recue :")); 
                   client.println(chaineAnalyse); 
                   client.println(F("Chaine non reconnue !")); 
                   client.println(F("Saisir chaine servo(xxx) avec xxx, un angle entre 0 et 180")); 
                  //-- une fois la réponse Ajax terminée, la fonction de callback drawData est exécutée
 
                } // fin else 

    else if (chaineRecue.startsWith("GET")) { // si la chaine recue commence par GET et pas une réponse précédente = on envoie page entiere

     Serial.println (F("Requete HTTP valide !"));

          envoiEnteteHTTP(client); // envoi entete HTTP OK 200 vers le client
     
     //--- la réponse HTML à afficher dans le navigateur

          //------ début de la page HTML ------- 
          client.println(F("<!DOCTYPE html>"));
          client.println(F("<html>"));

          //------- head = entete de la page HTML ----------          
         client.println(F("<head>"));

           client.println(F("<meta charset=\"utf-8\" />")); // fixe encodage caractères - utiliser idem dans navigateur
           client.println(F("<title>Titre</title>"));// titre de la page HTML

          //=============== bloc de code javascript ================           
             client.println(F("<!-- Début du code Javascript  -->")); 
             client.println(F("<script language=\"javascript\" type=\"text/javascript\">"));  
             client.println(F("<!--       "));

           // variables / objets globaux - a declarer avant les fonctions pour eviter problemes de portee
            client.println(F("var canvas= null;"));
            client.println(F("var contextCanvas = null;"));

            client.println(F("var textInputValeur=null;"));

            client.println(F("var textInputX=null;"));
            client.println(F("var textInputY=null;"));
            client.println(F("var textInputMin=null;"));
            client.println(F("var textInputMax=null;"));
            client.println(F("var textInputValeur=null;"));


            client.println(F(""));
  
             //----------------- Fonction principale éxécutée au chargement de la page ----------------
           
             client.println(F("window.onload = function () { // au chargement de la page"));

              
                client.println(F("canvas = document.getElementById(\"nomCanvas\");"));
                client.println(F("canvas.width = 500; // largeur canvas"));
                client.println(F("canvas.height = 20; // hauteur canvas"));

                //----- zone texte ----
                client.println(F("textarea = document.getElementById(\"textarea\"); // declare objet canvas a partir id = nom "));
                client.println(F("textarea.value=\"\";")); // efface le contenu 

                client.println(F("textInputX= document.getElementById(\"valeurX\");"));
                client.println(F("textInputY= document.getElementById(\"valeurY\");"));
                client.println(F("textInputMin= document.getElementById(\"valeurMin\");"));
                client.println(F("textInputMax= document.getElementById(\"valeurMax\");"));
                client.println(F("textInputValeur= document.getElementById(\"valeur\");"));
                client.println(F("textInputRacine= document.getElementById(\"racine\");"));
                client.println(F("textInputRacineFin= document.getElementById(\"racineFin\");"));

                client.println(F("textInputMin.value=\"0\";")); // posMin en degrés
                client.println(F("textInputMax.value=\"180\";")); // posMax en degrés
                client.println(F("textInputValeur.value=(Number(textInputMax.value)-Number(textInputMin.value))/2; "));
                client.println(F("textInputRacine.value=\"servo(\";")); // racine debut
                client.println(F("textInputRacineFin.value=\")\";")); // racine fin

              client.println(F("if (canvas.getContext){"));
  
                  client.println(F("contextCanvas = canvas.getContext(\"2d\"); "));
                  
                  // rect gris de la taille du canvas
                  client.println(F("contextCanvas.fillStyle = \"rgb(200,200,200)\"; "));
                  client.println(F("contextCanvas.fillRect (0, 0, canvas.width, canvas.height);"));
                  
                  // rect vert de la taille moitié du canvas                 
                  client.println(F("contextCanvas.fillStyle = \"rgb(0,255,0)\";"));
                  client.println(F("contextCanvas.fillRect (0, 0, canvas.width/2, canvas.height);"));

                  client.println(F("canvas.addEventListener('click',mouseClick,false); "));

                  client.println(F("} ")); // fin si canvas existe
                  
                  client.println(F("else {"));
                  client.println(F("alert(\"Canvas non disponible\");"));
                  client.println(F("} ")); // fin else

             client.println(F("} // fin onload"));

      client.println(F("function requeteAjax(chaineIn, callback) { ")); // debut envoi requete avec chaine personnalisee
            
               client.println(F("var xhr = XMLHttpRequest(); "));

               client.println(F("xhr.open(\"GET\", chaineIn, true);")); // envoi requete avec chaine personnalisee
               client.println(F("xhr.send(null);"));

               //------ gestion de l'évènement onreadystatechange ----- 
               client.println(F("xhr.onreadystatechange = function() { "));

                 client.println(F("if (xhr.readyState == 4 && xhr.status == 200) {"));

                   client.println(F("//alert(xhr.responseText);"));
                   client.println(F("callback(xhr.responseText);"));

                 client.println(F("} // fin if "));

               client.println(F("}; // fin function onreadystatechange"));
               //------ fin gestion de l'évènement onreadystatechange ----- 
               
             client.println(F("} // fin fonction requeteAjax"));


     client.println(F("function drawData(stringDataIn) { "));

              // ajoute la réponse au champ texte 
               client.println(F("textarea.value=textarea.value+stringDataIn;")); // ajoute la chaine au début - décale vers le bas...
               client.println(F("textarea.setSelectionRange(textarea.selectionEnd-1,textarea.selectionEnd-1) ;")); // se place à la fin -1 pour avant saut de ligne
              
            client.println(F("} // fin fonction drawData"));

            // ------------------------------ 

              //--- fonction de gestion du click souris sur le canvas ---
             client.println(F("function mouseClick(e) { "));// fonction recoit evenement obtenu par addEventListener
             
               client.println(F("var mouseX, mouseY;"));
               
               // firefox - recupere les coordonnees de la souris dans le canvas
               client.println(F("mouseX = e.layerX;"));
               client.println(F("mouseY = e.layerY;"));
               
               client.println(F("textInputX.value=mouseX; "));
               client.println(F("textInputY.value=mouseY;"));

               client.println(F("contextCanvas.fillStyle = \"rgb(200,200,200)\";"));
               client.println(F("contextCanvas.fillRect (0, 0, canvas.width, canvas.height);"));
               client.println(F("contextCanvas.fillStyle = \"rgb(0,255,0)\";"));
               client.println(F("contextCanvas.fillRect (0, 0, mouseX, canvas.height);"));
               
               // map valeur mouseX en valeur reelle            
               client.println(F("textInputValeur.value=Number(textInputMin.value)+Math.round((Number(textInputMax.value)-Number(textInputMin.value))*mouseX/canvas.width);"));
               client.println(F(""));
             
               // envoi de la requete Ajax
               client.println(F("requeteAjax(\"&\"+textInputRacine.value+textInputValeur.value+textInputRacineFin.value+\"=\", drawData);")); // envoi requete avec &chaine= et fonction de gestion resultat
              // utilise les champs racine et racineFin pour encadrer la valeur 
             client.println(F("}")); // mouseClick
            // ------------------------------ 
                 
             client.println(F("//-->"));  
             client.println(F("</script>")); 
             client.println(F("<!-- Fin du code Javascript --> ")); 
             
          //=============== fin du bloc de code javascript ================
          
          client.println(F("</head>"));
          //------- fin head = fin entete de la page HTML ----------          

           //------- body = corps de la page HTML ----------          
           client.println("<body>");           
           
           client.println(F("Serveur Arduino : Test envoi chaine par requete Ajax sur clic Slider Canvas"));
           client.println(F("<br/>"));
           client.println(F(""));
           client.println(F("<canvas id=\"nomCanvas\" width=\"300\" height=\"30\" style=\"position: relative;\"></canvas>"));
           client.println(F("<br />"));
           client.println(F("X=<input type=\"text\" id=\"valeurX\" />"));
           client.println(F("Y=<input type=\"text\" id=\"valeurY\" />"));
           
           client.println(F("<br />"));
           client.println(F("Valeur Min=<input type=\"text\" id=\"valeurMin\"  size=\"10\"/>"));
           client.println(F("Valeur Max=<input type=\"text\" id=\"valeurMax\" size=\"10\"/>"));

           client.println(F("<br />"));
           client.println(F("Racine : <input type=\"text\" id=\"racine\"  size=\"10\"/>"));
           client.println(F("Valeur Courante=<input type=\"text\" id=\"valeur\" size=\"10\"/>"));
           client.println(F("Fin : <input type=\"text\" id=\"racineFin\" size=\"5\"/>"));

           client.println(F("<br />"));
           client.println(F("En provenance du serveur Arduino :"));
           client.println(F("<br/>"));
           client.println(F("<textarea id=\"textarea\" rows=\"10\" cols=\"50\" > </textarea>")); // ajoute zone texte vide à la page 
           client.println(F("<br/>"));

           
           client.println(F("</body>"));           
          //------- fin body = fin corps de la page ----------          
            
         client.println(F("</html>"));
         //-------- fin de la page HTML -------      
     
    } // fin if GET


         else { // si la chaine recue ne commence pas par GET
     Serial.println (F("Requete HTTP non valide !"));    
    } // fin else
    
    //------ fermeture de la connexion ------ 
    
    // fermeture de la connexion avec le client après envoi réponse
    delay(1); // laisse le temps au client de recevoir la réponse 
    client.stop();
    Serial.println(F("------------ Fermeture de la connexion avec le client ------------")); // affiche le String de la requete
    Serial.println (F(""));
  
    } // --- fin if client connected
    
  } //---- fin if client ---- 


} // fin de la fonction loop()

    //------- fonctions communes ----- 

void envoiEnteteHTTP(EthernetClient clientIn){
 
 if (clientIn) {

   //-- envoi de la réponse HTTP --- 
           clientIn.println(F("HTTP/1.1 200 OK")); // entete de la réponse : protocole HTTP 1.1 et exécution requete réussie
           clientIn.println(F("Content-Type: text/html")); // précise le type de contenu de la réponse qui suit 
           clientIn.println(F("Connnection: close")); // précise que la connexion se ferme après la réponse
           clientIn.println(); // ligne blanche 
           
           //--- envoi en copie de la réponse http sur le port série 
           Serial.println(F("La reponse HTTP suivante est envoyee au client distant :")); 
           Serial.println(F("HTTP/1.1 200 OK"));
           Serial.println(F("Content-Type: text/html"));
           Serial.println(F("Connnection: close"));

 } // fin si client
 
} // fin envoiEnteteHTTP

