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

    // These variables are for optimizing the calculations to avoid repetitive operations.
    double m_sin_phi;
    double m_cos_phi;

    double m_sin_theta_s;
    double m_cos_theta_s;
    double m_tan_theta_s;

    double m_sin_theta_v;
    double m_cos_theta_v;
    double m_tan_theta_v;

    double m_delta_Val;
    double m_masse_Val;
    double m_cos_xsi_Val;
    double m_sin_xsi_Val;
    double m_xsi_Val;
    double m_cos_t_Val;
    double m_sin_t_Val;
    double m_t_Val;
    double m_FV_Val;
    double m_FR_Val;
};

#endif // DIRECTIONAL_MODEL
