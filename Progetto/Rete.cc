#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h" 
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/yans-wifi-helper.h"
#include <ns3/yans-wifi-phy.h>

#include <iostream>
#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

void RenamePcapFiles() {
    std::regex pattern("task-([0-9]+)-([0-9]+)-[0-9]+-[0-9]+\\.pcap");  // Regex per identificare il formato dei file
    for (const auto &entry : fs::directory_iterator(".")) {  // Itera su tutti i file nella directory corrente
        std::string filename = entry.path().filename().string();
        std::smatch matches;
        if (std::regex_match(filename, matches, pattern)) {
            if (matches.size() == 3) {
                // Costruisci il nuovo nome del file
                std::string newname = "task-" + matches[1].str() + "-" + matches[2].str() + ".pcap";
                // Rinomina il file
                fs::rename(entry.path(), entry.path().parent_path() / newname);
            }
        }
    }
}




//############################################################################
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Task_<1984820>");

int main (int argc, char* argv[])
{
//##########################################################################
std::string studentId = "nonInserito";  //1° task è un valore di default quando la matricola non viene passata da linea di comando
bool enableRtsCts = false; 		//2° task  RTS/CTS da implementare
bool tracing = false;		//3° task tracing da implementare
//#################################################################################

//##########################################################################
CommandLine cmd;	//serve per gestire i parametri da riga di comando
cmd.AddValue("studentId", "Student ID", studentId);
cmd.AddValue("enableRtsCts", "Enable RTS/CTS", enableRtsCts);
cmd.AddValue("tracing", "Enable promiscuous tracing", tracing);
//#################################################################################
cmd.Parse(argc, argv);

//##############################################################################
//std::cout/end servono per stampare su terminale e gestire flusso output 
if (studentId == "nonInserito") {	//se non è  stato passato niente su cmd
    std::cout << "Student ID non inserito digitare matricola!" << std::endl;
    return 0;
}

if (enableRtsCts){        //se passo via parametro enableRtsCts=true
Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(0));          //attivo RtsCts per ogni pacchetto > 0 byte
}

if (studentId != "1984820"){       //se la matricola non è quella del referente	
    std::cout << "StudentID non corretto, inserire matricola referente" << std::endl;
    return 0;
}

//######################################################################################

Time::SetResolution(Time::NS);
LogComponentEnable("UdpEchoClientApplication",LOG_LEVEL_INFO);
LogComponentEnable("UdpEchoServerApplication",LOG_LEVEL_INFO);

LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);

//CREAZIONE NODI #################################################################################################
NodeContainer p2pNodesRouter42;
p2pNodesRouter42.Create(2); //crea il router 4 e il router 2


NodeContainer p2pNodesRouter45;
p2pNodesRouter45.Create(1); // Crea il router 5
p2pNodesRouter45.Add(p2pNodesRouter42.Get(0)); // aggiunge al container il router 4 

//Creazione Client 6,7,8,9
NodeContainer p2pNodesClient6Router5;
p2pNodesClient6Router5.Create(1); // Crea il client 6
p2pNodesClient6Router5.Add(p2pNodesRouter45.Get(0)); // Aggiunge il router 5 al collegamento

NodeContainer p2pNodesClient7Router5;
p2pNodesClient7Router5.Create(1); // Crea il client 7
p2pNodesClient7Router5.Add(p2pNodesRouter45.Get(0)); // Aggiunge il router 5 al collegamento

NodeContainer p2pNodesClient8Router5;
p2pNodesClient8Router5.Create(1); // Crea il client 8
p2pNodesClient8Router5.Add(p2pNodesRouter45.Get(0)); // Aggiunge il router 5 al collegamento

NodeContainer p2pNodesClient9Router5;
p2pNodesClient9Router5.Create(1); // Crea il client 9
p2pNodesClient9Router5.Add(p2pNodesRouter45.Get(0)); // Aggiunge il router 5 al collegamento+

//Creazione nodo Server 3
NodeContainer p2pNodesServer3Router4;
p2pNodesServer3Router4.Create(1); // Crea il server 3
p2pNodesServer3Router4.Add(p2pNodesRouter42.Get(0)); // aggiunge al container il router 4  

//container per il collegamento point to point 
NodeContainer p2pNodesServer3Router5;
p2pNodesServer3Router5.Add(p2pNodesServer3Router4.Get(0)); // aggiunge al container il router 5
p2pNodesServer3Router5.Add(p2pNodesRouter45.Get(0));

// Creazione del nodo 10 e container connessione ptp tra il router 4 e il nodo wifi 10 
NodeContainer p2pNodeWifi10Router4;
p2pNodeWifi10Router4.Create(1);
p2pNodeWifi10Router4.Add(p2pNodesRouter42.Get(0)); // Assume router 4 è il primo nodo in p2pNodesRouter42

NodeContainer p2pNodeWifi10Server3;
p2pNodeWifi10Server3.Add(p2pNodeWifi10Router4.Get(0));
p2pNodeWifi10Server3.Add(p2pNodesServer3Router4.Get(0)); // Assumendo che il server 3 sia il primo nodo nel container

//Creazione nodo CSMA
NodeContainer csmaNodes012;
csmaNodes012.Create(2);    //crea il server 0 e il server 1
csmaNodes012.Add(p2pNodesRouter42.Get(1)); //inserisce il router 2 nella lan csma

// Creazione dei nodi Wi-Fi (client da 11 a 19)
NodeContainer wifiClientNodes11_19;
wifiClientNodes11_19.Create(9);
wifiClientNodes11_19.Add(p2pNodeWifi10Router4.Get(0));


//INSERIMENTO PARAMETRI #################################################################################################
PointToPointHelper pointToPointRouter42;
pointToPointRouter42.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));    //inserisce i parametri per la rete point to point tra il 
pointToPointRouter42.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));// router 2 e 4 

PointToPointHelper pointToPointRouter45;
pointToPointRouter45.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));    //inserisce i parametri per la rete point to point tra il 
pointToPointRouter45.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));// router 4 e 5 

//PTP per ogni Client (6,7,8,9)
PointToPointHelper pointToPointClient6Router5;
pointToPointClient6Router5.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPointClient6Router5.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

PointToPointHelper pointToPointClient7Router5;
pointToPointClient7Router5.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPointClient7Router5.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

PointToPointHelper pointToPointClient8Router5;
pointToPointClient8Router5.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPointClient8Router5.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

PointToPointHelper pointToPointClient9Router5;
pointToPointClient9Router5.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPointClient9Router5.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

//PTP tra il Server 3 e il Router 4
PointToPointHelper pointToPointServer3Router4;
pointToPointServer3Router4.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));    //inserisce i parametri per la rete point to point tra il 
pointToPointServer3Router4.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));// Server 3 e  Router 4 

//PTP tra il Server 3 e il Router 5
PointToPointHelper pointToPointServer3Router5;
pointToPointServer3Router5.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));    //inserisce i parametri per la rete point to point tra il 
pointToPointServer3Router5.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));// Server 3 e  Router 5 

// Collegamento Point-to-Point tra nodo wifi 10 e router 4
PointToPointHelper pointToPointNodeWifi10Router4;
pointToPointNodeWifi10Router4.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
pointToPointNodeWifi10Router4.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (20)));

// Configurazione Point-to-Point
PointToPointHelper pointToPointServer3NodeWifi10;
pointToPointServer3NodeWifi10.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
pointToPointServer3NodeWifi10.SetChannelAttribute("Delay", TimeValue(MilliSeconds(20)));

//Collegamento CSMA
CsmaHelper csma012;
csma012.SetChannelAttribute("DataRate", StringValue("10Mbps"));      //inserisce i parametri per la rete csma tra il router 2 e i server
csma012.SetChannelAttribute("Delay", TimeValue(MilliSeconds(200)));  // 0 e 1 



//CONFIGURAZIONE CANALE WIFI #################################################################################################

//Impostare Mobilità
MobilityHelper mobility;     //creiamo un oggetto di tipo mobilityhelper
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.Install (wifiClientNodes11_19); // Installiamo la mobility per i nodi wifi
//la mobility e per rendere la connessione wifi piu efficente

// Configurazione del canale Wi-Fi con ritardo di propagazione
YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");  

// Configurazione del livello fisico del Wi-Fi
YansWifiPhyHelper phy = YansWifiPhyHelper();
phy.SetChannel(channel.Create());
phy.Set("TxPowerStart", DoubleValue(20.0)); // dBm
phy.Set("TxPowerEnd", DoubleValue(20.0));   // dBm

// Configurazione Wi-Fi con bitrate fisso
WifiHelper wifi;    
wifi.SetStandard(WIFI_STANDARD_80211g);
wifi.SetRemoteStationManager("ns3::AarfWifiManager");     //algoritmo di controllo del tasso adattivo per le reti wireless


//INSTALLAZIONE COLLEGAMENTI #################################################################################################
NetDeviceContainer p2pDevicesRouter42;              //installazione del collegamento point to point tra il router 2 e 4 
p2pDevicesRouter42 = pointToPointRouter42.Install (p2pNodesRouter42);

NetDeviceContainer p2pDevicesRouter45;              //installazione del collegamento point to point tra il router 4 e 5
p2pDevicesRouter45 = pointToPointRouter45.Install (p2pNodesRouter45);

NetDeviceContainer csmaDevices012;           //istallazione del collegamento lan csma tra i server 0 e 1 e il router 2 
csmaDevices012 = csma012.Install (csmaNodes012);

//Installiamo i collegamenti dei Client 6,7,8,9
NetDeviceContainer p2pDevicesClient6Router5;
NetDeviceContainer p2pDevicesClient7Router5;
NetDeviceContainer p2pDevicesClient8Router5;
NetDeviceContainer p2pDevicesClient9Router5;
p2pDevicesClient6Router5 = pointToPointClient6Router5.Install(p2pNodesClient6Router5);
p2pDevicesClient7Router5 = pointToPointClient7Router5.Install(p2pNodesClient7Router5);
p2pDevicesClient8Router5 = pointToPointClient8Router5.Install(p2pNodesClient8Router5);
p2pDevicesClient9Router5 = pointToPointClient9Router5.Install(p2pNodesClient9Router5);

//Installiamo i collegamenti del Server 3
NetDeviceContainer p2pDevicesServer3Router4;              //installazione del collegamento point to point tra il Server 3 e il router 4
p2pDevicesServer3Router4 = pointToPointServer3Router4.Install (p2pNodesServer3Router4);

NetDeviceContainer p2pDevicesServer3Router5;              //installazione del collegamento point to point tra il Server 3 e il router 5
p2pDevicesServer3Router5 = pointToPointServer3Router5.Install (p2pNodesServer3Router5);

NetDeviceContainer p2pDevicesNodeWifi10Router4;           //istallazione collegamenteo poin to point tra il node wifi 10 e il router 4
p2pDevicesNodeWifi10Router4 = pointToPointNodeWifi10Router4.Install(p2pNodeWifi10Router4);

NetDeviceContainer p2pDevicesServer3NodeWifi10;           //istallazione collegamenteo poin to point tra il node wifi 10 e il Server 3
p2pDevicesServer3NodeWifi10 = pointToPointServer3NodeWifi10.Install(p2pNodeWifi10Server3);





//INSTALLAZIONE STACK DI INTERNET #################################################################################################
InternetStackHelper stack;
stack.Install(p2pNodesRouter42.Get(0)); // installa lo stack Internet sul router 4
stack.Install (csmaNodes012);   //installa pacchetto internet sui nodi della lan csma (cioè 0 1 e 2)
stack.Install (p2pNodesRouter42.Get(1)); // istallazione pacchetto internet sul router 2 
stack.Install(p2pNodesRouter45.Get(0)); //installazione pacchetto internet sul router 5

stack.Install(p2pNodesClient6Router5.Get(0)); // Installa lo stack Internet sul client 6
stack.Install(p2pNodesClient7Router5.Get(0)); // Installa lo stack Internet sul client 7
stack.Install(p2pNodesClient8Router5.Get(0)); // Installa lo stack Internet sul client 8
stack.Install(p2pNodesClient9Router5.Get(0)); // Installa lo stack Internet sul client 9

stack.Install(p2pNodesServer3Router4.Get(0)); //installazione pacchetto internet sul Server 3

stack.Install(wifiClientNodes11_19); //installazzione pacchetto internet sui client del wifi 

stack.Install(p2pNodeWifi10Server3); //installazione pacchetto internet sul collegamento p2p tra il node wifi 10 e il Server 3


//CONFIGURAZIONE LIVELLO MAC PER IL CLIENT E NODO AP WIFI ###########################################################################

// Configurazione del livello MAC per i client Wi-Fi
WifiMacHelper mac;
mac.SetType("ns3::AdhocWifiMac");       //Settiamo un MAC tipo ad hoc per i client Wi-Fi
NetDeviceContainer clientDevices11_19 = wifi.Install(phy, mac, wifiClientNodes11_19); // Installa il livello MAC sui client Wi-Fi


//INSERIMENTO INDIRIZZI #################################################################################################
//Usiamo degli IP con gamma 192.168.x.x perche stiamo creando una piccola rete che richiede pochi indirizzi (es: rete domestica)
Ipv4AddressHelper addressRouter42;                            //assegna indirizzi per la sottorete tra i router 4 e 2 
addressRouter42.SetBase("192.168.1.0", "255.255.255.252");
Ipv4InterfaceContainer p2pRouter42Interfaces = addressRouter42.Assign(p2pDevicesRouter42);

Ipv4AddressHelper addressCsma012;               //assegna indirizzi per la sottorete corrispondente alla lan csma
addressCsma012.SetBase("192.168.1.32", "255.255.255.248");
Ipv4InterfaceContainer csmaDevices012Interfaces = addressCsma012.Assign(csmaDevices012);

Ipv4AddressHelper addressRouter45;          //assegna indirizzi per la sottorete tra i router 4 e 5 
addressRouter45.SetBase("192.168.1.4", "255.255.255.252");
Ipv4InterfaceContainer p2pRouter45Interfaces = addressRouter45.Assign(p2pDevicesRouter45);

//IP Client 6,7,8,9
//maschera /30 perche per collegamenti p2p non servono piu di due IP
Ipv4AddressHelper addressClient6Router5;        //assegna indirizzi per la sottorete tra il router 5 e il client 6 
addressClient6Router5.SetBase("192.168.1.16", "255.255.255.252");
Ipv4InterfaceContainer p2pInterfacesClient6Router5 = addressClient6Router5.Assign(p2pDevicesClient6Router5);

Ipv4AddressHelper addressClient7Router5;        //assegna indirizzi per la sottorete tra il router 5 e il client 7 
addressClient7Router5.SetBase("192.168.1.20", "255.255.255.252");
Ipv4InterfaceContainer p2pInterfacesClient7Router5 = addressClient7Router5.Assign(p2pDevicesClient7Router5);

Ipv4AddressHelper addressClient8Router5;        //assegna indirizzi per la sottorete tra il router 5 e il client 8 
addressClient8Router5.SetBase("192.168.1.24", "255.255.255.252");
Ipv4InterfaceContainer p2pInterfacesClient8Router5 = addressClient8Router5.Assign(p2pDevicesClient8Router5);

Ipv4AddressHelper addressClient9Router5;        //assegna indirizzi per la sottorete tra il router 5 e il client 9 
addressClient9Router5.SetBase("192.168.1.28", "255.255.255.252");
Ipv4InterfaceContainer p2pInterfacesClient9Router5 = addressClient9Router5.Assign(p2pDevicesClient9Router5);

//IP Server 3
Ipv4AddressHelper addressServer3Router4;          //assegna indirizzi per la sottorete tra il Server 3 e il router 4 
addressServer3Router4.SetBase("192.168.1.8", "255.255.255.252");
Ipv4InterfaceContainer p2pRouter34Interfaces = addressServer3Router4.Assign(p2pDevicesServer3Router4);

Ipv4AddressHelper addressServer3Router5;          //assegna indirizzi per la sottorete tra il Server 3 e il router 5 
addressServer3Router5.SetBase("192.168.1.12", "255.255.255.252");
Ipv4InterfaceContainer p2pServer3Router5Interfaces = addressServer3Router5.Assign(p2pDevicesServer3Router5);

Ipv4AddressHelper addressNodeWifi10Router4;       // Assegnazione per il collegamento point-to-point tra nodo 10 e router 4
addressNodeWifi10Router4.SetBase("192.168.1.36", "255.255.255.252");
Ipv4InterfaceContainer p2p10Interfaces = addressNodeWifi10Router4.Assign(p2pDevicesNodeWifi10Router4);

//IP dei client Wifi 11-19
Ipv4AddressHelper addressNodeWifi10Clients11_19;
addressNodeWifi10Clients11_19.SetBase("192.168.1.48", "255.255.255.240");
Ipv4InterfaceContainer wifiNode10Clients11_19Interfaces = addressNodeWifi10Clients11_19.Assign(clientDevices11_19);

//IP collegamento server 3 e node wifi 10
Ipv4AddressHelper addressServer3NodeWifi10;
addressServer3NodeWifi10.SetBase("192.168.1.40", "255.255.255.252");
Ipv4InterfaceContainer p2pServer3NodeWifi10Interfaces = addressServer3NodeWifi10.Assign(p2pDevicesServer3NodeWifi10);


// ************************************************************
// Creiamo un oggetto Ipv4GlobalRoutingHelper per configurare le tabelle di routing IPv4
Ipv4GlobalRoutingHelper::PopulateRoutingTables();


//INSTALLAZIONI ULTERIORI PER INVIO PACCHETTI
//PARTE UDP: ##########################################################################################
//il mio obiettivo è ora cercare di inviare un pacchetto tramite udp dal client 6 al server 0, lo faccio per sperimentare che la rete funzioni 

UdpEchoServerHelper echoServer(30);              // Installa Echo Server sul server 3, e imposta parametri
ApplicationContainer serverAppsUdp0 = echoServer.Install(p2pNodesServer3Router4.Get(0));
serverAppsUdp0.Start(Seconds(1.0));
serverAppsUdp0.Stop(Seconds(15.0));

Ipv4Address serverAddress = p2pServer3Router5Interfaces.GetAddress(0);   // Ottieni l'indirizzo IP del server 3

UdpEchoClientHelper echoClient(serverAddress, 30);    

//Impostazione parametri per il client 7
echoClient.SetAttribute("MaxPackets", UintegerValue(250));  
echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(20)));
echoClient.SetAttribute("PacketSize", UintegerValue(1336));

ApplicationContainer clientApps7 = echoClient.Install(p2pNodesClient7Router5.Get(0));
clientApps7.Start(Seconds(2.0));
clientApps7.Stop(Seconds(15.0));

//Settiamo il payload per il client 7
std::string groupInfo = "Francesco,Torella,1984820,Dario,Neris Di Santo,1985793,Tommaso,Rinaldi,1983126,Federico,Zaza,1990588,Camilla,Sed,2008300";

// Aggiungi zeri fino a raggiungere una lunghezza di 1336 byte
while (groupInfo.size() < 1335) {
    groupInfo += '0';
}

for (ApplicationContainer::Iterator i = clientApps7.Begin(); i != clientApps7.End(); ++i) {
    Ptr<Application> app = (*i);
    echoClient.SetFill(app, groupInfo);
}


//PARTE TCP:
//################################################################################################################################################################################################

// Definizione delle porte per le connessioni TCP
uint16_t portServer0 = 8080; // Porta per il Server 0
uint16_t portServer1 = 8081; // Porta per il Server 1

// Impostazione del PacketSink sui Server "destinazione per i pacchetti in arrivo"
PacketSinkHelper packetSinkHelperServer0("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), portServer0));
PacketSinkHelper packetSinkHelperServer1("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), portServer1));

ApplicationContainer serverAppsTcp0 = packetSinkHelperServer0.Install(csmaNodes012.Get(0)); // Installa su Server 0
ApplicationContainer serverAppsTcp1 = packetSinkHelperServer1.Install(csmaNodes012.Get(1)); // Installa su Server 1

serverAppsTcp0.Start(Seconds(0.1));
serverAppsTcp0.Stop(Seconds(15.0));
serverAppsTcp1.Start(Seconds(0.1));
serverAppsTcp1.Stop(Seconds(15.0));

// Ottieni gli indirizzi IP dei server dalla loro interfaccia CSMA
Ipv4Address server0Address = csmaDevices012Interfaces.GetAddress(0);
Ipv4Address server1Address = csmaDevices012Interfaces.GetAddress(1);

// Impostazione del BulkSendApp per Nodo 14
BulkSendHelper bulkSendHelperNode14("ns3::TcpSocketFactory", InetSocketAddress(server0Address, portServer0));
bulkSendHelperNode14.SetAttribute("MaxBytes", UintegerValue(1166 * 1024 * 1024));
ApplicationContainer clientAppsNode14 = bulkSendHelperNode14.Install(wifiClientNodes11_19.Get(3)); // Nodo 14
clientAppsNode14.Start(Seconds(0.27));
clientAppsNode14.Stop(Seconds(15.0));

// Impostazione del BulkSendApp per Nodo 12
BulkSendHelper bulkSendHelperNode12("ns3::TcpSocketFactory", InetSocketAddress(server1Address, portServer1));
bulkSendHelperNode12.SetAttribute("MaxBytes", UintegerValue(1882 * 1024 * 1024));
ApplicationContainer clientAppsNode12 = bulkSendHelperNode12.Install(wifiClientNodes11_19.Get(1)); // Nodo 12
clientAppsNode12.Start(Seconds(3.42));
clientAppsNode12.Stop(Seconds(15.0));  

// Impostazione del BulkSendApp per Nodo 9
BulkSendHelper bulkSendHelperNode9("ns3::TcpSocketFactory", InetSocketAddress(server0Address, portServer0));
bulkSendHelperNode9.SetAttribute("MaxBytes", UintegerValue(1700 * 1024 * 1024));
ApplicationContainer clientAppsNode9 = bulkSendHelperNode9.Install(p2pNodesClient9Router5.Get(0)); // Nodo 9
clientAppsNode9.Start(Seconds(3.33));
clientAppsNode9.Stop(Seconds(15.0));



//TRACING:##################################################################################################################################################################################
 

if (tracing) {

    // Tracing TCP per i server utilizzando i NetDevices corrispondenti
    std::string traceFileNode2Server0 = "task-2-0";
    csma012.EnablePcap(traceFileNode2Server0, csmaDevices012.Get(0), true);
 


    std:: string traceFileNode5Router4 = "task-5-4";
    pointToPointRouter45.EnablePcap(traceFileNode5Router4, p2pDevicesRouter45.Get(1),true);

    std:: string traceFileNode10Router4 = "task-10-4";
    pointToPointNodeWifi10Router4.EnablePcap(traceFileNode10Router4, p2pDevicesNodeWifi10Router4.Get(1), true);

    std:: string traceFileRouter5Server3 = "task-5-3";
    pointToPointServer3Router5.EnablePcap(traceFileRouter5Server3, p2pDevicesServer3Router5.Get(1), true);

    std:: string traceFileRouter4Router2 = "task-4-2";
    pointToPointRouter42.EnablePcap(traceFileRouter4Router2, p2pDevicesRouter42.Get(1), true);

    std:: string traceFileNode5Client9 = "task-5-9";
    pointToPointClient9Router5.EnablePcap(traceFileNode5Client9, p2pDevicesClient9Router5.Get(0), true);

    std:: string traceFileNode5Client7 = "task-5-7";
    pointToPointClient7Router5.EnablePcap(traceFileNode5Client7, p2pDevicesClient7Router5.Get(0), true);


    std:: string traceFileWifi10Client14 = "task-10-14";
    phy.EnablePcap(traceFileWifi10Client14, clientDevices11_19.Get(3), true);

}


//#################################################################################################################################################################################################

//SIMULAZIONE 
Simulator::Stop(Seconds(15.0));
Simulator::Run();
Simulator::Destroy();


if (tracing) {
    RenamePcapFiles();
}


return 0;
}