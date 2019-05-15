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

#include "WStringToString.h"

#ifdef _WIN32
std::string ConvertFromUtf16ToUtf8(const wchar_t * wstr)
{
	std::string convertedString;
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, 0, 0, 0, 0);
	if(requiredSize > 0)
	{
		std::vector<char> buffer(requiredSize);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &buffer[0], requiredSize, 0, 0);
		convertedString.assign(buffer.begin(), buffer.end() - 1);
	}
	return convertedString;
}
#endif

