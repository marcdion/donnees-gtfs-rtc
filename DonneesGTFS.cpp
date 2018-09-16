//
// Created by Mario Marchand on 16-12-29.
//

#include "DonneesGTFS.h"

using namespace std;

//! \brief construit un objet GTFS
//! \param[in] p_date: la date utilisée par le GTFS
//! \param[in] p_now1: l'heure du début de l'intervalle considéré
//! \param[in] p_now2: l'heure de fin de l'intervalle considéré
//! \brief Ces deux heures définissent l'intervalle de temps du GTFS; seuls les moments de [p_now1, p_now2) sont considérés
DonneesGTFS::DonneesGTFS(const Date &p_date, const Heure &p_now1, const Heure &p_now2)
        : m_date(p_date), m_now1(p_now1), m_now2(p_now2), m_nbArrets(0), m_tousLesArretsPresents(false)
{
}

//! \brief partitionne un string en un vecteur de strings
//! \param[in] s: le string à être partitionner
//! \param[in] delim: le caractère utilisé pour le partitionnement
//! \return le vecteur de string sans le caractère utilisé pour le partitionnement
vector<string> DonneesGTFS::string_to_vector(const string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> elems;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

//! \brief ajoute les lignes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les lignes
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterLignes(const std::string &p_nomFichier)
{
    // declaration de la variable de type fstream
    std::fstream fichierRoutes;

    // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
    fichierRoutes.open(p_nomFichier, ios_base :: in);
    if (!fichierRoutes.fail()){
        string strUneLigne;
        string delim = ",";
        vector<string> vLigne;

        unsigned int uiRouteId;

        string sRouteShortName;
        string sRouteDesc;
        CategorieBus sRouteType;

        getline(fichierRoutes, strUneLigne);
        while(getline(fichierRoutes, strUneLigne)){
            vLigne = string_to_vector(strUneLigne, *delim.c_str());
            uiRouteId = (unsigned int) stoi(vLigne[0]);
            sRouteShortName = vLigne[2].substr(1, vLigne[2].size() - 2);

            sRouteDesc = vLigne[4].substr(1, vLigne[4].size() - 2);
            sRouteType = Ligne::couleurToCategorie(vLigne[7]);

            Ligne *uneLigne = new Ligne(uiRouteId, sRouteShortName, sRouteDesc, sRouteType);
            m_lignes.insert({uneLigne->getId(), *uneLigne});
            m_lignes_par_numero.insert({uneLigne->getNumero(), *uneLigne});
        }
        fichierRoutes.close();
    }else{
        fichierRoutes.close();
    }
}

//! \brief ajoute les stations dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterStations(const std::string &p_nomFichier)
{
    // declaration de la variable de type fstream
    std::fstream fichierStations;

    // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
    fichierStations.open(p_nomFichier, ios_base :: in);
    if (!fichierStations.fail()){
        string strUneStation;
        string delim = ",";
        vector<string> vStation;

        unsigned int iStationId;

        string sStationNom;
        string sStationDescription;

        getline(fichierStations, strUneStation);
        while(getline(fichierStations, strUneStation)){
            vStation = string_to_vector(strUneStation, *delim.c_str());
            iStationId = (unsigned int) stoi(vStation[0]);
            sStationNom = vStation[1].substr(1, vStation[1].size() - 2);

            sStationDescription = vStation[2].substr(1, vStation[2].size() - 2);

            Coordonnees *cStationCoordonees = new Coordonnees(stoi(vStation[3]), stoi(vStation[4]));
            Station *uneStation = new Station(iStationId, sStationNom, sStationDescription, *cStationCoordonees);
            m_stations.insert({uneStation->getId(), *uneStation});

        }
        fichierStations.close();
    }else{
        fichierStations.close();
    }
}

//! \brief ajoute les transferts dans l'objet GTFS
//! \breif Cette méthode doit âtre utilisée uniquement après que tous les arrêts ont été ajoutés
//! \brief les transferts (entre stations) ajoutés sont uniquement ceux pour lesquelles les stations sont prensentes dans l'objet GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les station
//! \throws logic_error si un problème survient avec la lecture du fichier
//! \throws logic_error si tous les arrets de la date et de l'intervalle n'ont pas été ajoutés
void DonneesGTFS::ajouterTransferts(const std::string &p_nomFichier)
{
    if(m_tousLesArretsPresents){
        // declaration de la variable de type fstream
        std::fstream fichierTransfers;

        // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
        fichierTransfers.open(p_nomFichier, ios_base :: in);
        if (!fichierTransfers.fail()){
            string strUnTransfert;
            string delim = ",";
            vector<string> vTransfert;

            unsigned int uiFromStationId;
            unsigned int uiToStationId;
            unsigned int uiMinTransferTime;

            getline(fichierTransfers, strUnTransfert);
            while(getline(fichierTransfers, strUnTransfert)){
                vTransfert = string_to_vector(strUnTransfert, *delim.c_str());

                uiFromStationId = (unsigned int) stoi(vTransfert[0]);
                uiToStationId = (unsigned int) stoi(vTransfert[1]);

                if(m_stations.count(uiFromStationId) != 0){
                    if(m_stations.count(uiToStationId) != 0){
                        if((unsigned int) stoi(vTransfert[3]) == 0){
                            uiMinTransferTime = 1;
                        }else{
                            uiMinTransferTime = (unsigned int) stoi(vTransfert[3]);
                        }

                        auto tupleTransfert = make_tuple(uiFromStationId, uiToStationId, uiMinTransferTime);
                        m_transferts.push_back(tupleTransfert);
                    }
                }
            }
            fichierTransfers.close();
        }else{
            fichierTransfers.close();
        }
    }
}


//! \brief ajoute les services de la date du GTFS (m_date)
//! \param[in] p_nomFichier: le nom du fichier contenant les services
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterServices(const std::string &p_nomFichier)
{
// declaration de la variable de type fstream
    std::fstream fichierServices;

    // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
    fichierServices.open(p_nomFichier, ios_base :: in);
    if (!fichierServices.fail()){
        string strUnService;
        string delim = ",";
        vector<string> vService;

        string iServiceId;

        string sDateTemporaire;
        Date dServiceDate;
        unsigned int uiServiceExceptionType;

        getline(fichierServices, strUnService);
        while(getline(fichierServices, strUnService)){
            vService = string_to_vector(strUnService, *delim.c_str());
            iServiceId = vService[0];

            dServiceDate = *new Date((unsigned int) stoi(vService[1].substr(0, 4)), (unsigned int) stoi(vService[1].substr(4,2)), (unsigned int) stoi(vService[1].substr(6,2)));
            uiServiceExceptionType = (unsigned int) stoi(vService[2]);

            if(uiServiceExceptionType == 1){
                if(dServiceDate == m_date){
                    m_services.insert(iServiceId);
                }
            }
        }
        fichierServices.close();
    }else{
        fichierServices.close();
    }
}

//! \brief ajoute les voyages de la date
//! \brief seuls les voyages dont le service est présent dans l'objet GTFS sont ajoutés
//! \param[in] p_nomFichier: le nom du fichier contenant les voyages
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterVoyagesDeLaDate(const std::string &p_nomFichier)
{
    // declaration de la variable de type fstream
    std::fstream fichierVoyages;

    // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
    fichierVoyages.open(p_nomFichier, ios_base :: in);
    if (!fichierVoyages.fail()){
        string strUnVoyage;
        string delim = ",";
        vector<string> vVoyage;

        string sVoyageId;

        unsigned int uiLigne;
        string sServiceId;
        string sDestination;

        getline(fichierVoyages, strUnVoyage);
        while(getline(fichierVoyages, strUnVoyage)){
            vVoyage = string_to_vector(strUnVoyage, *delim.c_str());

            uiLigne = (unsigned int) stoi(vVoyage[0]);
            sServiceId = vVoyage[1];
            sVoyageId = vVoyage[2];
            sDestination = vVoyage[3];

            sDestination = sDestination.substr(1, sDestination.size() - 2);

            if(m_services.count(sServiceId) != 0) {
                Voyage unVoyage = *new Voyage(sVoyageId, uiLigne, sServiceId, sDestination);
                m_voyages.insert({unVoyage.getId(), unVoyage});
            }
        }
        fichierVoyages.close();
    }else{
        fichierVoyages.close();
    }
}

//! \brief ajoute les arrets aux voyages présents dans le GTFS si l'heure du voyage appartient à l'intervalle de temps du GTFS
//! \brief De plus, on enlève les voyages qui n'ont pas d'arrêts dans l'intervalle de temps du GTFS
//! \brief De plus, on enlève les stations qui n'ont pas d'arrets dans l'intervalle de temps du GTFS
//! \param[in] p_nomFichier: le nom du fichier contenant les arrets
//! \post assigne m_tousLesArretsPresents à true
//! \throws logic_error si un problème survient avec la lecture du fichier
void DonneesGTFS::ajouterArretsDesVoyagesDeLaDate(const std::string &p_nomFichier)
{
    // declaration de la variable de type fstream
    std::fstream fichierArretsDeVoyages;

    // Overture du fichier p_nomFichier (placé dans la repertoire courant) en lecture binaire.
    fichierArretsDeVoyages.open(p_nomFichier, ios_base :: in);
    if (!fichierArretsDeVoyages.fail()){
        string strUnArretVoyage;
        string delim = ",";
        string delimHeures = ":";
        vector<string> vArret;

        unsigned int uiStationId;       //m_station_id;
        unsigned int uiNumeroSequence;  //m_numero_sequence;
        unsigned int nbArrets = 0;

        Heure hHeureArrivee;    //m_heure_arrivee;
        Heure hHeureDepart;     //m_heure_depart;
        string sVoyageId;       //m_voyage_id;

        getline(fichierArretsDeVoyages, strUnArretVoyage);
        while(getline(fichierArretsDeVoyages, strUnArretVoyage)){
            vArret = string_to_vector(strUnArretVoyage, *delim.c_str());

            sVoyageId = vArret[0];

            if(m_voyages.count(sVoyageId) != 0){
                hHeureArrivee = *new Heure((unsigned int) stoi(string_to_vector(vArret[1], *delimHeures.c_str())[0]), (unsigned int) stoi(string_to_vector(vArret[1], *delimHeures.c_str())[1]), (unsigned int) stoi(string_to_vector(vArret[1], *delimHeures.c_str())[2]));
                hHeureDepart = *new Heure((unsigned int) stoi(string_to_vector(vArret[2], *delimHeures.c_str())[0]), (unsigned int) stoi(string_to_vector(vArret[2], *delimHeures.c_str())[1]), (unsigned int) stoi(string_to_vector(vArret[2], *delimHeures.c_str())[2]));

                uiStationId = (unsigned int) stoi(vArret[3]);
                uiNumeroSequence = (unsigned int) stoi(vArret[4]);

                if(hHeureDepart >= m_now1){
                    if(hHeureDepart < m_now2){
                        Arret::Ptr a_ptr = make_shared<Arret>(uiStationId, hHeureArrivee, hHeureDepart, uiNumeroSequence, sVoyageId);
                        m_voyages[sVoyageId].ajouterArret(a_ptr);
                        m_stations[uiStationId].addArret(a_ptr);

                        nbArrets++;
                    }
                }

            }
        }

        for (auto it = m_voyages.cbegin(); it != m_voyages.cend();) {
            if (it->second.getNbArrets() == 0) {
                it = m_voyages.erase(it);
            }else{
                ++it;
            }
        }

        for (auto it = m_stations.cbegin(); it != m_stations.cend();) {
            if (it->second.getNbArrets() == 0) {
                it = m_stations.erase(it);
            }else{
                ++it;
            }
        }

        m_nbArrets = nbArrets;
        m_tousLesArretsPresents = true;

        fichierArretsDeVoyages.close();
    }else{
        fichierArretsDeVoyages.close();
    }
}

unsigned int DonneesGTFS::getNbArrets() const
{
    return m_nbArrets;
}

size_t DonneesGTFS::getNbLignes() const
{
    return m_lignes.size();
}

size_t DonneesGTFS::getNbStations() const
{
    return m_stations.size();
}

size_t DonneesGTFS::getNbTransferts() const
{
    return m_transferts.size();
}

size_t DonneesGTFS::getNbServices() const
{
    return m_services.size();
}

size_t DonneesGTFS::getNbVoyages() const
{
    return m_voyages.size();
}

void DonneesGTFS::afficherLignes() const
{
    std::cout << "======================" << std::endl;
    std::cout << "   LIGNES GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_lignes.size() << "   " << std::endl;
    std::cout << "======================" << std::endl;
    for (const auto & ligneM : m_lignes_par_numero)
    {
        cout << ligneM.second;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   STATIONS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_stations.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (const auto & stationM : m_stations)
    {
        std::cout << stationM.second << endl;
    }
    std::cout << std::endl;
}

void DonneesGTFS::afficherTransferts() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   TRANSFERTS GTFS   " << std::endl;
    std::cout << "   COMPTE = " << m_transferts.size() << "   " << std::endl;
    std::cout << "========================" << std::endl;
    for (unsigned int i = 0; i < m_transferts.size(); ++i)
    {
        std::cout << "De la station " << get<0>(m_transferts.at(i)) << " vers la station " << get<1>(m_transferts.at(i))
        <<
        " en " << get<2>(m_transferts.at(i)) << " secondes" << endl;
        
    }
    std::cout << std::endl;
    
}


void DonneesGTFS::afficherArretsParVoyages() const
{
    std::cout << "=====================================" << std::endl;
    std::cout << "   VOYAGES DE LA JOURNÉE DU " << m_date << std::endl;
    std::cout << "   " << m_now1 << " - " << m_now2 << std::endl;
    std::cout << "   COMPTE = " << m_voyages.size() << "   " << std::endl;
    std::cout << "=====================================" << std::endl;
    
    for (const auto & voyageM : m_voyages)
    {
        unsigned int ligne_id = voyageM.second.getLigne();
        auto l_itr = m_lignes.find(ligne_id);
        cout << (l_itr->second).getNumero() << " ";
        cout << voyageM.second << endl;
        for (const auto & a: voyageM.second.getArrets())
        {
            unsigned int station_id = a->getStationId();
            auto s_itr = m_stations.find(station_id);
            std::cout << a->getHeureArrivee() << " station " << s_itr->second << endl;
        }
    }
    
    std::cout << std::endl;
}

void DonneesGTFS::afficherArretsParStations() const
{
    std::cout << "========================" << std::endl;
    std::cout << "   ARRETS PAR STATIONS   " << std::endl;
    std::cout << "   Nombre d'arrêts = " << m_nbArrets << std::endl;
    std::cout << "========================" << std::endl;
    for ( const auto & stationM : m_stations)
    {
        std::cout << "Station " << stationM.second << endl;
        for ( const auto & arretM : stationM.second.getArrets())
        {
            string voyage_id = arretM.second->getVoyageId();
            auto v_itr = m_voyages.find(voyage_id);
            unsigned int ligne_id = (v_itr->second).getLigne();
            auto l_itr = m_lignes.find(ligne_id);
            std::cout << arretM.first << " - " << (l_itr->second).getNumero() << " " << v_itr->second << std::endl;
        }
    }
    std::cout << std::endl;
}

const std::map<std::string, Voyage> &DonneesGTFS::getVoyages() const
{
    return m_voyages;
}

const std::map<unsigned int, Station> &DonneesGTFS::getStations() const
{
    return m_stations;
}

const std::vector<std::tuple<unsigned int, unsigned int, unsigned int> > &DonneesGTFS::getTransferts() const
{
    return m_transferts;
}

Heure DonneesGTFS::getTempsFin() const
{
    return m_now2;
}

Heure DonneesGTFS::getTempsDebut() const
{
    return m_now1;
}

const std::unordered_map<unsigned int, Ligne> &DonneesGTFS::getLignes() const
{
    return m_lignes;
}



