#include <math.h>

//---get P value from F value and df1 df2---------------------------start
double c[11] = { 0.0000677106, -0.0003442342, 0.0015397681, -0.0024467480,
        0.0109736958, -0.0002109075, 0.0742379071, 0.0815782188, 0.4118402518,
        0.4227843370, 1.0000000000 };

double gammaln(double xx) {
    double x, y, tmp, ser;
    static const double cof[6] = {
        76.18009172947146,
        -86.50532032941677,
        24.0140982408091,
        -1.231739572460155,
        0.1208650973866179e-2,
        -0.5395239384953e-5
    };
    y = x = xx;
    tmp = (x + 0.5) * log(x + 5.5) - (x + 5.5);
    ser = 1.000000000190015;
    for (int j = 0; j < 6; j ++) {
        ser += cof[j] / (y + 1);
        y = y + 1;
    }
    return tmp + log(2.5066282746310005 * ser / x);
}

double beta(double x, double y) {
    if (x <= 0 || y <= 0) {
        return 0;
    }
    return exp(gammaln(x)+gammaln(y)-gammaln(x + y));
}

double abslt(double x) {
    if (x < 0.0)
        return -x;
    else
        return x;
}

double fi(int N, double x, double a, double b) {
    int n = N / 2;
    double f = 0.0, f1, s1, s2, tmpU, tmpV;
    int i;
    for (i = n; i >= 1; i--) {
        tmpU = (a + 2.0 * i - 1.0) * (a + 2.0 * i);
        s2 = i * (b - i) * x / tmpU;
        f1 = s2 / (1.0 + f);
        tmpV = (a + 2.0 * i - 2.0) * (a + 2.0 * i - 1.0);
        s1 = -(a + i - 1.0) * (b + a + i - 1.0) * x / tmpV;
        f = s1 / (1.0 + f1);
    }
    return 1.0 / (1.0 + f);
}

double incomBeta(double x, double a, double b) {
    if (a <= 0.0 || b <= 0.0) {
        return 0.0;
    }
    if (abslt(x - 0.0) < 1.0e-30 || abslt(x - 1.0) < 1.0e-30) {
        return 0.0;
    }

    double c1, c2, c3, f1, f2;
    int n;
    c1 = pow(x, a);
    c2 = pow(1.0 - x, b);
    c3 = beta(a, b);
    if (x < (a + 1.0) / (a + b + 2.0)) {
        n = 1;
        while (1) {
            f1 = fi(2 * n, x, a, b);
            f2 = fi(2 * n + 2, x, a, b);
            if (abslt(f2 - f1) < 1.0e-30)
                return f2 * c1 * c2 / a / c3;
            else
                n++;
        }
    } else {
        if (abslt(x - 0.5) < 1.0e-30 && abslt(a - b) < 1.0e-30)
            return 0.5;
        else {
            n = 1;
            while (1) {
                f1 = fi(2 * n, 1.0 - x, b, a);
                f2 = fi(2 * n + 2, 1.0 - x, b, a);
                if (abslt(f2 - f1) < 1.0e-30)
                    return 1.0 - f2 * c1 * c2 / b / c3;
                else
                    n++;
            }
        }
    }
    return 0;
}

double getPvalue(double f, double n1, double n2) {
    if (f < 0.0)
        f = -f;
    return incomBeta(n2 / (n2 + n1 * f), n2 / 2.0, n1 / 2.0);
}

//double FDist(double F,double m,double n)
//   {
//       double xx,p;

//       if(m<=0 || n<=0) p=-1;
//       else if(F>0)
//       {
//           xx=F/(F+n/m);
//           p=betainc(xx,m/2,n/2);
//       }
//       return(1-p);
//   }

//---get P value from F value and df1 df2---------------------------end

//    int main()
//    {
//      double y;
//      y=FDist(304.94,17,18);
//      std::cout << "P-value =" << std::endl;
//      std::cout << y << std::endl;
//      system("pause");
//    }
