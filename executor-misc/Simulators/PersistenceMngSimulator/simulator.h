#ifndef SIMULATOR_H
#define SIMULATOR_H


class PersistenceManager;

class Simulator
{
public:
    Simulator();
    ~Simulator();

private:
    PersistenceManager *m_pPersistenceMng;
};

#endif // SIMULATOR_H
