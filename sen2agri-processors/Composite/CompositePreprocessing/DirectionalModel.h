#ifndef DIRECTIONAL_MODEL_H
#define DIRECTIONAL_MODEL_H


class DirectionalModel
{
public:
    DirectionalModel(double theta_s, double phi_s, double theta_v, double phi_v);
    double delta();
    double masse();
    double cos_xsi();
    double sin_xsi();
    double xsi();
    double cos_t();
    double sin_t();
    double t();
    double FV();
    double FR();
    double dir_mod(double kV,double kR);

private:
    double m_theta_s;
    double m_theta_v;
    double m_phi;
};

#endif // DIRECTIONAL_MODEL
