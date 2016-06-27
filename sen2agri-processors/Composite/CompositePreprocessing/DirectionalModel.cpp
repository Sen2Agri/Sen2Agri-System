/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#include <math.h>
#include <algorithm>    // std::min, std::max
#include "DirectionalModel.h"

#define xsi_0 (1.5*M_PI/180.)

DirectionalModel::DirectionalModel(double theta_s, double phi_s, double theta_v, double phi_v)
{
    m_theta_s = theta_s * M_PI / 180;
    m_theta_v = theta_v * M_PI / 180;
    m_phi = (phi_s - phi_v) * M_PI / 180;

    m_sin_theta_s = sin(m_theta_s);
    m_cos_theta_s = cos(m_theta_s);
    m_tan_theta_s = tan(m_theta_s);

    m_sin_theta_v = sin(m_theta_v);
    m_cos_theta_v = cos(m_theta_v);
    m_tan_theta_v = tan(m_theta_v);

    m_sin_phi = sin(m_phi);
    m_cos_phi = cos(m_phi);

    m_delta_Val = delta();
    m_masse_Val = masse();
    m_cos_xsi_Val = cos_xsi();
    m_sin_xsi_Val = sin_xsi();
    m_xsi_Val = xsi();
    m_cos_t_Val = cos_t();
    m_sin_t_Val = sin_t();
    m_t_Val = t();
    m_FV_Val = FV();
    m_FR_Val = FR();
}

double DirectionalModel::delta() {
    double delta = sqrt(m_tan_theta_s*m_tan_theta_s + m_tan_theta_v*m_tan_theta_v - 2*m_tan_theta_s*m_tan_theta_v*m_cos_phi);
    return delta;
}

// Air Mass
double DirectionalModel::masse() {
    double  masse=1/m_cos_theta_s+1/m_cos_theta_v;
    return masse;
}

double DirectionalModel::cos_xsi() {
    double cos_xsi=m_cos_theta_s*m_cos_theta_v + m_sin_theta_s*m_sin_theta_v*m_cos_phi;
    return cos_xsi;
}

double DirectionalModel::sin_xsi() {
    double x=m_cos_xsi_Val;
    double sin_xsi=sqrt(1 - x*x);
    return sin_xsi;
}

double DirectionalModel::xsi() {
    double xsi=acos(m_cos_xsi_Val);
    return xsi;
}

//#Function t
double DirectionalModel::cos_t() {
    double trig=m_tan_theta_s*m_tan_theta_v*m_sin_phi;
    double coef=1;  //#Coef=1 looks good, but Bréon et Vermote use Coef=2;
    double cos_t=std::min(std::max(coef/m_masse_Val*sqrt(m_delta_Val*m_delta_Val + trig*trig),(double)-1),(double)1);
    return cos_t;
}

double DirectionalModel::sin_t() {
    double x=m_cos_t_Val;
    double sin_t=sqrt(1 - x*x);
    return sin_t;
}

double DirectionalModel::t() {
    //#print 'theta_v %f cos_t %F'%(m_theta_v*180/M_PI,cos_t())
    double t=acos(m_cos_t_Val);
    return t;
}

// #function FV Ross_Thick,  V stands for Volume
double DirectionalModel::FV() {

    double FV=m_masse_Val/M_PI*(m_t_Val - m_sin_t_Val*m_cos_t_Val - M_PI) + (1+m_cos_xsi_Val)/2/m_cos_theta_s/m_cos_theta_v;
    return FV;
}

// #function FR  Li-Sparse, R stands for Roughness
double DirectionalModel::FR() {
    double A=1/(m_cos_theta_s+m_cos_theta_v);

    double FR=4/3./M_PI*A*((M_PI/2-m_xsi_Val)*m_cos_xsi_Val+m_sin_xsi_Val)*(1+1/(1+m_xsi_Val/xsi_0))- 1./3;
    return FR;
}

double DirectionalModel::dir_mod(double kV,double kR) {
    double rho=1 +kV*m_FV_Val + kR*m_FR_Val;
    return rho;
}
