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
}

double DirectionalModel::delta() {
    double delta = sqrt(tan(m_theta_s)*tan(m_theta_s) + tan(m_theta_v)*tan(m_theta_v) - 2*tan(m_theta_s)*tan(m_theta_v)*cos(m_phi));
    return delta;
}

// Air Mass
double DirectionalModel::masse() {
    double  masse=1/cos(m_theta_s)+1/cos(m_theta_v);
    return masse;
}

double DirectionalModel::cos_xsi() {
    double cos_xsi=cos(m_theta_s)*cos(m_theta_v) + sin(m_theta_s)*sin(m_theta_v)*cos(m_phi);
    return cos_xsi;
}

double DirectionalModel::sin_xsi() {
    double x=cos_xsi();
    double sin_xsi=sqrt(1 - x*x);
    return sin_xsi;
}

double DirectionalModel::xsi() {
    double xsi=acos(cos_xsi());
    return xsi;
}

//#Function t
double DirectionalModel::cos_t() {
    double trig=tan(m_theta_s)*tan(m_theta_v)*sin(m_phi);
    double d=delta();
    double coef=1;  //#Coef=1 looks good, but Bréon et Vermote use Coef=2;
    double cos_t=std::min(std::max(coef/masse()*sqrt(d*d + trig*trig),(double)-1),(double)1);
    return cos_t;
}

double DirectionalModel::sin_t() {
    double x=cos_t();
    double sin_t=sqrt(1 - x*x);
    return sin_t;
}

double DirectionalModel::t() {
    //#print 'theta_v %f cos_t %F'%(m_theta_v*180/M_PI,cos_t())
    double t=acos(cos_t());
    return t;
}

// #function FV Ross_Thick,  V stands for Volume
double DirectionalModel::FV() {

    double FV=masse()/M_PI*(t() - sin_t()*cos_t() - M_PI) + (1+cos_xsi())/2/cos(m_theta_s)/cos(m_theta_v);
    return FV;
}

// #function FR  Li-Sparse, R stands for Roughness
double DirectionalModel::FR() {
    double A=1/(cos(m_theta_s)+cos(m_theta_v));

    double FR=4/3./M_PI*A*((M_PI/2-xsi())*cos_xsi()+sin_xsi())*(1+1/(1+xsi()/xsi_0))- 1./3;
    return FR;
}

double DirectionalModel::dir_mod(double kV,double kR) {
    double rho=1 +kV*FV() + kR*FR();
    return rho;
}
