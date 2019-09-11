//**********************************************************
// TITLE       :DWT CODEC CLASS IMPLEMENTATION FILE
// VERSION     :1.0
// FILE        :Dwtcodec.cpp
// DESCRIPTION :A class for implementing a discrete wavelet
//              transform on images. This is a derived class 
//              from the abstract class of type CODEC.
// DATE        :December 1997
// AUTHOR      :K.L.Ferguson
//**********************************************************
#include "stdafx.h"

#include "dwtcodec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Divide by root 2.
#define R2 (double)(1.41421356)

///////////////////////////////////////////////////////////////
// Neural Network 6 hidden layers lifting filter 9 - 7.
///////////////////////////////////////////////////////////////
//static double Pred_6Hid_7[] =
//{ 
//   -0.1073,   -0.1531,   -0.0265,    0.0268,
//   -0.0368,    0.0512,    0.1174,   -0.0942,
//    0.0087,   -0.4490,   -0.3892,    0.0514,
//   -0.1030,    0.5999,    0.5903,   -0.0658,
//   -0.1037,    0.1501,    0.3144,    0.0817,
//   -0.0039,   -0.3275,   -0.3459,    0.0565
//};
//static double Pred_Lin_7[] =
//  {-0.1287,    0.1126,   -0.6445,    1.0125,    0.3393,   -0.4884};
//
//static double Update_6Hid_9[] =
//  {0.0007,    0.0549,   -0.5620,   -0.5453,    0.0558,    0.0015};

static double Pred_6Hid_7[] =
{ 
   -0.1221,   -0.1590,   -0.0120,   -0.0002,
   -0.0179,    0.0327,    0.1008,   -0.0817,
   -0.0509,   -0.4185,   -0.3145,   -0.0327,
   -0.0669,    0.5421,    0.4729,    0.0101,
   -0.0615,    0.1224,    0.2581,    0.1301,
   -0.0600,   -0.3004,   -0.2840,   -0.0157
};
static double Pred_Lin_7[] =
  {-0.1452,    0.0675,   -0.6292,    1.0762,    0.2847,   -0.4469};
static double Update_6Hid_9[] =
  {0.0119,   -0.0392,   -0.4400,   -0.5063,    0.0666,   -0.0427};

static NNFILTERTYPE NN_6HID_9_7 =
  { Pred_6Hid_7,6,4,
    Pred_Lin_7,6,
    Update_6Hid_9,6};

///////////////////////////////////////////////////////////////
// Neural Network 16 hidden layers lifting filter 9 - 7.
///////////////////////////////////////////////////////////////
static double Pred_16Hid_7[] =
{ 
    0.0719,    0.1420,    0.0029,    0.0076,
    0.0054,    0.0076,   -0.1667,    0.0050,
   -0.0530,    0.0220,   -0.0617,   -0.0100,
   -0.0664,    0.3878,    0.3132,    0.0478,
   -0.1024,   -0.0459,    0.0429,    0.0142,
    0.0951,    0.2580,    0.1566,   -0.0343,
    0.0334,   -0.2668,   -0.1528,    0.0045,
    0.0043,   -0.2257,   -0.2701,   -0.0586,
    0.0694,   -0.1989,   -0.2757,   -0.0443,
    0.0348,   -0.0615,   -0.0756,   -0.0920,
   -0.0771,   -0.0516,    0.0338,    0.0663,
    0.0611,   -0.2906,   -0.1752,   -0.0741,
    0.0389,    0.3955,    0.4145,    0.0006,
   -0.0809,   -0.0219,    0.0440,    0.0791,
    0.0007,   -0.1897,   -0.2437,    0.0673,
    0.0331,    0.3385,    0.2011,    0.0560
};
static double Pred_16Lin_7[] =
{
   0.1008,   -0.0988,   -0.0303,   0.4917,
  -0.0217,    0.2971,   -0.2709,  -0.3455,
  -0.3088,   -0.0968,   -0.0201,  -0.3189,
   0.6400,    0.0089,   -0.2721,   0.4003
};
static double Update_16Hid_9[] =
  {-0.0151,    0.0082,   -0.5253,   -0.5301,    0.0493,   -0.0018};

static NNFILTERTYPE NN_16HID_9_7 =
  { Pred_16Hid_7,16,4,
    Pred_16Lin_7,16,
    Update_16Hid_9,6};

///////////////////////////////////////////////////////////////
// Neural Network 8 hidden layers lifting filter 13 - 11.
///////////////////////////////////////////////////////////////

static double Pred_8Hid_11[] =
{ 
   -0.1087,   -0.1016,    0.0977,    0.0136,    0.0223,    0.0686,
   -0.0469,    0.0163,    0.2888,    0.1849,    0.0694,    0.0021,
   -0.0589,    0.0666,   -0.1603,   -0.1210,    0.0474,   -0.0789,
   -0.0335,   -0.0058,    0.5213,    0.5645,   -0.0431,   -0.0262,
   -0.0688,   -0.0762,    0.0148,    0.0448,   -0.0717,    0.1010,
    0.0322,    0.0267,    0.5155,    0.3806,    0.0109,   -0.0269,
   -0.0572,    0.0014,   -0.2077,   -0.2469,   -0.0411,   -0.1155,
   -0.0103,    0.0249,    0.0143,    0.0843,   -0.0827,    0.0319
};
static double Pred_Lin_11[] =
  {0.0586,    0.3095,   -0.1833,    0.9245,    0.0306,    0.7061,   -0.3466,0.0533};
static double Update_8Hid_13[] =
  {-0.0076,   -0.0163,    0.0493,   -0.4966,   -0.5305,    0.0453,   -0.0064,-0.0041};

static NNFILTERTYPE NN_8HID_13_11 =
  { Pred_8Hid_11,8,6,
    Pred_Lin_11,8,
    Update_8Hid_13,8};

///////////////////////////////////////////////////////////////
// Fast lifting filter 5 - 3.
///////////////////////////////////////////////////////////////
static double Pred_3[] =
  {0.000233,0.250305,
   0.250305,0.000233};
static double Update_5[] =
  {0.004429,-0.494926,
   -0.494926,0.004429};

static FASTFILTERTYPE LIFTING_5_3 =
  { Pred_3,4,
    Update_5,4};

///////////////////////////////////////////////////////////////
// Fast lifting filter 9 - 7.
///////////////////////////////////////////////////////////////
static double Pred_7[] =
  {0.003004,-0.048980,0.295988,
   0.295988,-0.048980,0.003004};
static double Update_9[] =
  {-0.011330,0.073860,-0.593200,
   -0.593200,0.073860,-0.011330};

static FASTFILTERTYPE LIFTING_9_7 =
  { Pred_7,6,
    Update_9,6};

///////////////////////////////////////////////////////////////
// Fast lifting filter 13 - 11.
///////////////////////////////////////////////////////////////
static double Pred_11[] =
  {-0.003725,0.016356,-0.078071,0.314976,
   0.314976,-0.078071,0.016356,-0.003725};
static double Update_13[] =
  {0.019007,-0.036437,0.115639,-0.626038,
   -0.626038,0.115639,-0.036437,0.019007};

static FASTFILTERTYPE LIFTING_13_11 =
  { Pred_11,8,
    Update_13,8};

///////////////////////////////////////////////////////////////
// Asymmetric filter 5 - 3.
///////////////////////////////////////////////////////////////
static double h_asym_5[] =
  {-0.125,0.25,
   0.75,
   0.25,-0.125};
static double inv_g_asym_5[] = //Coeff. reversed.
  {0.125,0.25,-0.75,
   0.25,
   0.125};
static double inv_g_even_asym_5[] = //Coeff. reversed.
  {0.25,
   0.25,
   };
static double inv_g_odd_asym_5[] = //Coeff. reversed.
  {0.125,-0.75,
   0.125};
static double inv_h_asym_3[] =
  {0.25,
   0.5,
   0.25};
static double inv_h_even_asym_3[] = //Coeff. reversed.
  {
   0.5
   };
static double inv_h_odd_asym_3[] = //Coeff. reversed.
  {0.25,
   0.25};
static double g_asym_3[] =
  {
   0.25,
   -0.5,0.25};

static FILTERTYPE ASYMMETRIC_5_3 =
  { h_asym_5,2,5,
    g_asym_3,0,3,
    inv_h_asym_3,1,3,
    inv_g_asym_5,3,5,
    inv_h_even_asym_3,0,1,
    inv_h_odd_asym_3,0,2,
    inv_g_even_asym_5,1,2,
    inv_g_odd_asym_5,1,3};

///////////////////////////////////////////////////////////////
// Spline filter 9 - 7.
///////////////////////////////////////////////////////////////
static double h_spline_II[] =
  {0.026749,-(0.016864),-(0.078223),0.266864,
   0.602949,
   0.266864,-(0.078223),-(0.016864),0.026749};
static double inv_g_spline_II[] = //Coeff. reversed.
  {-0.026749,-0.016864,0.078223,0.266864,-0.602949,
   0.266864,
   0.078223,-0.016864,-0.026749};
static double inv_g_even_spline_II[] = //Coeff. reversed.
  {-0.016864,0.266864,
   0.266864,
   -0.016864};
static double inv_g_odd_spline_II[] = //Coeff. reversed.
  {-0.026749,0.078223,-0.602949,
   0.078223,-0.026749};
static double inv_h_spline_II[] = //Coeff. reversed.
  {-(0.045636),-(0.028772),0.295636,
   0.557543,
   0.295636,-(0.028772),-(0.045636)};
static double inv_h_even_spline_II[] = //Coeff. reversed.
  {-0.028772,
   0.557543,
   -0.028772};
static double inv_h_odd_spline_II[] = //Coeff. reversed.
  {-0.045636,0.295636,
   0.295636,-0.045636};
static double g_spline_II[] =
  {-0.045636,0.028772,
   0.295636,
   -0.557543,0.295636,0.028772,-0.045636};

static FILTERTYPE SPLINE_II =
  { h_spline_II,4,9,
    g_spline_II,2,7,
    inv_h_spline_II,3,7,
    inv_g_spline_II,5,9,
    inv_h_even_spline_II,1,3,
    inv_h_odd_spline_II,1,4,
    inv_g_even_spline_II,2,4,
    inv_g_odd_spline_II,2,5};

///////////////////////////////////////////////////////////////
// Asymmetric filter 13 - 11.
///////////////////////////////////////////////////////////////
static double h_asym_13[] =
  {-0.00599,0.002658,0.033433,-0.023670,-0.048704,0.271012,
   0.542524,
   0.271012,-0.048704,-0.023670,0.033433,0.002658,-0.00599};
static double inv_g_asym_13[] = //Coeff. reversed.
  {0.00599,0.002658,-0.033433,-0.023670,0.048704,0.271012,-0.542524,
   0.271012,
   0.048704,-0.023670,-0.033433,0.002658,0.00599};
static double inv_g_even_asym_13[] = //Coeff. reversed.
  {0.002658,-0.023670,0.271012,
   0.271012,
   -0.023670,0.002658};
static double inv_g_odd_asym_13[] = //Coeff. reversed.
  {0.00599,-0.033433,0.048704,-0.542524,
   0.048704,-0.033433,0.00599};
static double inv_h_asym_11[] =
  {0.010028,0.004449,-0.076889,-0.048906,0.316861,
   0.588912,
   0.316861,-0.048906,-0.076889,0.004449,0.010028};
static double inv_h_even_asym_11[] = //Coeff. reversed.
  {0.004449,-0.048906,
   0.588912,
   -0.048906,0.004449};
static double inv_h_odd_asym_11[] = //Coeff. reversed.
  {0.010028,-0.076889,0.316861,
   0.316861,-0.076889,0.010028};
static double g_asym_11[] =
  {0.010028,-0.004449,-0.076889,0.048906,
   0.316861,
   -0.588912,0.316861,0.048906,-0.076889,-0.004449,0.010028};

static FILTERTYPE ASYMMETRIC_13_11 =
  { h_asym_13,6,13,
    g_asym_11,4,11,
    inv_h_asym_11,5,11,
    inv_g_asym_13,7,13,
    inv_h_even_asym_11,2,5,
    inv_h_odd_asym_11,2,6,
    inv_g_even_asym_13,3,6,
    inv_g_odd_asym_13,3,7};

///////////////////////////////////////////////////////////////
// Symmetric filter 6 - 10.
///////////////////////////////////////////////////////////////

static double h_sym_6[] =
  {-0.129078/R2,0.047699/R2,
   0.788486/R2,
   0.788486/R2,0.047699/R2,-0.129078/R2};
static double inv_g_sym_6[] = //Coeff. reversed.
  {0.129078/R2,0.047699/R2,-0.788486/R2,
   0.788486/R2,
   -0.047699/R2,-0.129078/R2};
static double inv_g_even_sym_6[] = //Coeff. reversed.
  {0.047699/R2,
   0.788486/R2,
   -0.129078/R2};
static double inv_g_odd_sym_6[] = //Coeff. reversed.
  {0.129078/R2,-0.788486/R2,
   -0.047699/R2};
static double inv_h_sym_10[] = //Coeff. reversed.
  {0.018914/R2,0.006989/R2,-0.067237/R2,0.133389/R2,0.615015/R2,
   0.615015/R2,
   0.133389/R2,-0.067237/R2,0.006989/R2,0.018914/R2};
static double inv_h_even_sym_10[] = //Coeff. reversed.
  {0.006989/R2,0.133389/R2,
   0.615015/R2,
   -0.067237/R2,0.018914/R2};
static double inv_h_odd_sym_10[] = //Coeff. reversed.
  {0.018914/R2,-0.067237/R2,0.615015/R2,
   0.133389/R2,0.006989/R2};
static double g_sym_10[] =
  {0.018914/R2,-0.006989/R2,-0.067237/R2,-0.133389/R2,
   0.615015/R2,
   -0.615015/R2,0.133389/R2,0.067237/R2,0.006989/R2,-0.018914/R2};

static FILTERTYPE SYMMETRIC_6_10 =
  { h_sym_6,2,6,
    g_sym_10,4,10,
    inv_h_sym_10,5,10,
    inv_g_sym_6,3,6,
    inv_h_even_sym_10,2,5,
    inv_h_odd_sym_10,2,5,
    inv_g_even_sym_6,1,3,
    inv_g_odd_sym_6,1,3};

///////////////////////////////////////////////////////////////
// Symmetric filter 18 - 18.
///////////////////////////////////////////////////////////////

static double h_sym_18[] =
  {0.0012243/R2,-0.0006986/R2,-0.01183749/R2,0.01168591/R2,
   0.07130977/R2,-0.03099791/R2,-0.22632564/R2,0.06927336/R2,
   0.73184426/R2,
   0.73184426/R2,0.06927336/R2,-0.22632564/R2,-0.03099791/R2,
   0.07130977/R2,0.01168591/R2,-0.01183749/R2,-0.0006986/R2,
   0.0012243/R2};
static double inv_g_sym_18[] = //Coeff. reversed.
  {-0.0012243/R2,
   -0.0006986/R2,0.01183749/R2,0.01168591/R2,-0.07130977/R2,
   -0.03099791/R2,0.22632564/R2,0.06927336/R2,-0.73184426/R2,
   0.73184426/R2,
   -0.06927336/R2,-0.22632564/R2,0.03099791/R2,0.07130977/R2,
   -0.01168591/R2,-0.01183749/R2,0.0006986/R2,0.0012243/R2};
static double inv_g_even_sym_18[] = //Coeff. reversed.
  {-0.0006986/R2,0.01168591/R2,
   -0.03099791/R2,0.06927336/R2,
   0.73184426/R2,
   -0.22632564/R2,0.07130977/R2,
   -0.01183749/R2,0.0012243/R2};
static double inv_g_odd_sym_18[] = //Coeff. reversed.
  {-0.0012243/R2,
   0.01183749/R2,-0.07130977/R2,
   0.22632564/R2,-0.73184426/R2,
   -0.06927336/R2,0.03099791/R2,
   -0.01168591/R2,0.0006986/R2};
static double inv_h_sym_18[] = //Coeff. reversed.
  {0.0012243/R2,
   0.00069979/R2,-0.01134887/R2,-0.01141245/R2,0.02347331/R2,
   0.00174835/R2,-0.04441890/R2,0.20436993/R2,0.64790805/R2,
   0.64790805/R2,
   0.20436993/R2,-0.04441890/R2,0.00174835/R2,0.02347331/R2,
   -0.01141245/R2,-0.01134887/R2,0.00069979/R2,0.0012243/R2};
static double inv_h_even_sym_18[] = //Coeff. reversed.
  {0.00069979/R2,-0.01141245/R2,
   0.00174835/R2,0.20436993/R2,
   0.64790805/R2,
   -0.04441890/R2,0.02347331/R2,
   -0.01134887/R2,0.0012243/R2};
static double inv_h_odd_sym_18[] = //Coeff. reversed.
  {0.0012243/R2,
   -0.01134887/R2,0.02347331/R2,
   -0.04441890/R2,0.64790805/R2,
   0.20436993/R2,0.00174835/R2,
   -0.01141245/R2,0.00069979/R2};
static double g_sym_18[] =
  {0.0012243/R2,-0.00069979/R2,-0.01134887/R2,0.01141245/R2,
   0.02347331/R2,-0.00174835/R2,-0.04441890/R2,-0.20436993/R2,
   0.64790805/R2,
   -0.64790805/R2,0.20436993/R2,0.04441890/R2,0.00174835/R2,
   -0.02347331/R2,-0.01141245/R2,0.01134887/R2,0.00069979/R2,
   -0.0012243/R2};

static FILTERTYPE SYMMETRIC_18 =
  { h_sym_18,8,18,
    g_sym_18,8,18,
    inv_h_sym_18,9,18,
    inv_g_sym_18,9,18,
    inv_h_even_sym_18,4,9,
    inv_h_odd_sym_18,4,9,
    inv_g_even_sym_18,4,9,
    inv_g_odd_sym_18,4,9};

///////////////////////////////////////////////////////////////
// Symmetric filter 24 - 20.
///////////////////////////////////////////////////////////////

#define R22 (double)(2.0)
//#define R22 (double)(2.0 * 1.41421356)

static double h_sym_24[] =
  {0.00133565,-0.00201229,-0.00577577,
   0.00863853,0.01279957,-0.02361445,
   -0.01900854,0.04320273,-0.00931630,
   -0.12180846,0.05322182,
   0.41589714,
   0.41589714,0.05322182,-0.12180846,
   -0.00931630,0.04320273,-0.01900854,
   -0.02361445,0.01279957,0.00863853,
   -0.00577577,-0.00201229,0.00133565};
static double inv_g_sym_24[] = //Coeff. reversed.
  {0.00133565/R22,0.00201229/R22,-0.00577577/R22,
   -0.00863853/R22,0.01279957/R22,0.02361445/R22,
   -0.01900854/R22,-0.04320273/R22,-0.00931630/R22,
   0.12180846/R22,0.05322182/R22,-0.41589714/R22,
   0.41589714/R22,
   -0.05322182/R22,-0.12180846/R22,
   0.00931630/R22,0.04320273/R22,0.01900854/R22,
   -0.02361445/R22,-0.01279957/R22,0.00863853/R22,
   0.00577577/R22,-0.00201229/R22,-0.00133565/R22};
static double inv_g_even_sym_24[] = //Coeff. reversed.
  {0.00133565/R22,-0.00577577/R22,0.01279957/R22,
   -0.01900854/R22,-0.00931630/R22,0.05322182/R22,
   0.41589714/R22,
   -0.12180846/R22,0.04320273/R22,
   -0.02361445/R22,0.00863853/R22,-0.00201229/R22};
static double inv_g_odd_sym_24[] = //Coeff. reversed.
  {0.00201229/R22,-0.00863853/R22,0.02361445/R22,
   -0.04320273/R22,0.12180846/R22,-0.41589714/R22,
   -0.05322182/R22,0.00931630/R22,0.01900854/R22,
   -0.01279957/R22,0.00577577/R22,-0.00133565/R22};
static double inv_h_sym_20[] = //Coeff. reversed.
  {0.00465997/R22,0.00702071/R22,-0.01559987/R22,
   -0.02327921/R22,0.05635238/R22,0.10021543/R22,
   -0.06596151/R22,-0.13387993/R22,0.38067810/R22,1.10398118/R22,
   1.10398118/R22,
   0.38067810/R22,-0.13387993/R22,-0.06596151/R22,
   0.10021543/R22,0.05635238/R22,-0.02327921/R22,
   -0.01559987/R22,0.00702071/R22,0.00465997/R22};
static double inv_h_even_sym_20[] = //Coeff. reversed.
  {0.00465997/R22,-0.01559987/R22,
   0.05635238/R22,-0.06596151/R22,0.38067810/R22,
   1.10398118/R22,
   -0.13387993/R22,0.10021543/R22,-0.02327921/R22,
   0.00702071/R22};
static double inv_h_odd_sym_20[] = //Coeff. reversed.
  {0.00702071/R22,-0.02327921/R22,0.10021543/R22,
   -0.13387993/R22,1.10398118/R22,
   0.38067810/R22,-0.06596151/R22,
   0.05635238/R22,-0.01559987/R22,0.00465997/R22};
static double g_sym_20[] =
  {-0.00465997,0.00702071,0.01559987,
   -0.02327921,-0.05635238,0.10021543,
   0.06596151,-0.13387993,-0.38067810,
   1.10398118,
   -1.10398118,0.38067810,
   0.13387993,-0.06596151,-0.10021543,
   0.05635238,0.02327921,-0.01559987,
   -0.00702071,0.00465997};

static FILTERTYPE SYMMETRIC_24_20 =
  { h_sym_24,11,24,
    g_sym_20,9,20,
    inv_h_sym_20,10,20,
    inv_g_sym_24,12,24,
    inv_h_even_sym_20,5,10,
    inv_h_odd_sym_20,4,10,
    inv_g_even_sym_24,6,12,
    inv_g_odd_sym_24,5,12};

///////////////////////////////////////////////////////////////
// Public Implementations.

CDWTCODEC::CDWTCODEC()
{
  hWk = NULL;
  Wk = NULL;

  CurrDwtParam.WaveletType = STD_FILTER_9_7;
  CurrDwtParam.ConvType = BOUNDARY_REFLECTION;
  CurrDwtParam.LumLevels = 3;
  CurrDwtParam.ChrLevels = 3;
  F = SPLINE_II;
  Ff = LIFTING_9_7;
  Fn = NN_6HID_9_7;
}//end CDWTCODEC Constructor.

CDWTCODEC::~CDWTCODEC()
{
}//end CDWTCODEC Destructor.

void CDWTCODEC::SetParameters(void *Params)
{
  DWT_TYPE *p;

  p = (DWT_TYPE *)Params;

  CurrDwtParam = *p;
}//end SetParameters.

int CDWTCODEC::OpenCODEC(void * Img)
{
  CImage *I;
  I = (CImage *)Img;

	if(I->m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	error = 1;//ONLY YUV IMAGES MAY BE CODED.
  	  return(0);
		}//end if m_ExtraColorType...

  //Determine the YUV data sizes.
  int YLine = I->GetYWidth();

	//Allocate working memory for 4 lines of YUV results max.
	hWk = GlobalAlloc(GMEM_FIXED,(4 * YLine * sizeof(int)));
	if(!hWk)
  {
    error = 2;//WORKING MEMORY UNAVAILABLE.
	  return(0);
  }//end if !hWk...
	Wk = (int *)GlobalLock(hWk);

  return(1);
}//end OpenCODEC.

int CDWTCODEC::CODE(void *Img)
{
  CImage *I;
  I = (CImage *)Img;

	if(I->m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	error = 1;//ONLY YUV IMAGES MAY BE CODED.
  	  return(0);
		}//end if m_ExtraColorType...

  //Determine the YUV data sizes.
	int YLine,YRow,UVLine,UVRow;
	YLine = I->GetYWidth();
	YRow = I->GetYHeight();
	UVLine = I->GetUVWidth();
	UVRow = I->GetUVHeight();
  //Do convolution along rows.
  int R = YRow;
  int C = YLine;
  int i,j,m;
  int *Low,*High,*Data;

////////////////////////////////////////////////////////
// TESTING.
////////////////////////////////////////////////////////
//  //Extract a piece of the 1st line for processing. LEN pixels.
//  #define LEN 50
//
//  int D[LEN];
//  int LowPass[LEN];
//  int HighPass[LEN];
//  int InvLowPass[LEN];
//  int InvHighPass[LEN];
//  int Result[LEN];
//
//  for(i = 0; i < LEN; i++)
//    {
//      D[i] = I->m_Y[i];
//      LowPass[i] = 0;
//      HighPass[i] = 0;
//      InvLowPass[i] = 0;
//      InvHighPass[i] = 0;
//      Result[i] = 0;
//    }//end for i...
//
//  switch(CurrWltType)
//  {
//    case STD_FILTER_6_10:
//      F = SYMMETRIC_6_10;
//      DWT_1D(D,LEN,LowPass,HighPass);
//      IDWT_1D(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//    case STD_FILTER_18_18:
//      F = SYMMETRIC_18;
//      DWT_1D(D,LEN,LowPass,HighPass);
//      IDWT_1D(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//    case STD_FILTER_24_20:
//      F = SYMMETRIC_24_20;
//      DWT_1D(D,LEN,LowPass,HighPass);
//      IDWT_1D(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//    case STD_FILTER_9_7:
//      F = SPLINE_II;
//      DWT_1D(D,LEN,LowPass,HighPass);
//      IDWT_1D(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//    case FAST_LINEAR:
//      FAST_DWT_1D_LINEAR(D,LEN,LowPass,HighPass);
//      FAST_IDWT_1D_LINEAR(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//    case FAST_CUBIC_4:
//      FAST_DWT_1D_CUBIC4(D,LEN,LowPass,HighPass);
//      FAST_IDWT_1D_CUBIC4(LowPass,HighPass,LEN,InvLowPass,InvHighPass,Result);
//      break;
//  }//end switch CurrWltType...
//
//  // Show Up sample.
//  for(i = (LEN - 1); i > 0; i-=2)
//    {
//      LowPass[i] = 0;
//      LowPass[i-1] = LowPass[i/2];
//      HighPass[i] = 0;
//      HighPass[i-1] = HighPass[i/2];
//    }//end for i...
//
//  //Store results.
//	CStdioFile Fl;
//	CString Line;
//	int CheckFile;
//	CheckFile = Fl.Open("Dwt.dat",CStdioFile::modeCreate | 
//		CStdioFile::modeWrite | CStdioFile::typeText);
//
//  for(i = 0; i < LEN; i++)
//  {
//		if(CheckFile)
//		{
//		  Line = "";
//      Line.Format(_T("%d,%d,%d,%d,%d,%d,%d\n"),
//        i,D[i],(int)LowPass[i],(int)HighPass[i],
//        (int)InvLowPass[i],(int)InvHighPass[i],Result[i]);
//		  Fl.WriteString(Line);
//    }//end if CeckFile...
//  }//end for int i...
//
//	if(CheckFile)
//	  Fl.Close();
//
////////////////////////////////////////////////////////
// END TESTING.
////////////////////////////////////////////////////////

  //Set the filter parameters.
  SetFilter(CurrDwtParam.WaveletType);

  for(m = 0; m < CurrDwtParam.LumLevels; m++)
  {
    //Luminance.
    R = YRow >> m;
    C = YLine >> m;
    //In x direction.
    for(i = 0; i < R; i++)
    {
      Low = Wk;
      High = (int *)(Wk + (C/2));
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,
          (int *)((I->m_Y) + i*YLine),C,Low,High);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_Y) + i*YLine + j) = Low[j];
    }//end for i...
  
    //In y direction.
    for(j = 0; j < C; j++)
    {
      Low = Wk;
      High = (int *)(Wk + (R/2));
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_Y) + i*YLine + j);
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,Data,R,Low,High);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_Y) + i*YLine + j) = Low[i];
    }//end for j...
  }//end for m...
  
  for(m = 0; m < CurrDwtParam.ChrLevels; m++)
  {
    //Do U colour component.
    R = UVRow >> m;
    C = UVLine >> m;
    //In x direction.
    for(i = 0; i < R; i++)
    {
      Low = Wk;
      High = (int *)(Wk + (C/2));
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,
          (int *)((I->m_U) + i*UVLine),C,Low,High);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_U) + i*UVLine + j) = Low[j];
    }//end for i...
  
    //In y direction.
    for(j = 0; j < C; j++)
    {
      Low = Wk;
      High = (int *)(Wk + (R/2));
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_U) + i*UVLine + j);
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,Data,R,Low,High);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_U) + i*UVLine + j) = Low[i];
    }//end for j...
  
    //Do V colour component.
    //In x direction.
    for(i = 0; i < R; i++)
    {
      Low = Wk;
      High = (int *)(Wk + (C/2));
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,
          (int *)((I->m_V) + i*UVLine),C,Low,High);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_V) + i*UVLine + j) = Low[j];
    }//end for i...
  
    //In y direction.
    for(j = 0; j < C; j++)
    {
      Low = Wk;
      High = (int *)(Wk + (R/2));
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_V) + i*UVLine + j);
      //Do the 1-D transform.
      Dwt(CurrDwtParam.WaveletType,Data,R,Low,High);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_V) + i*UVLine + j) = Low[i];
    }//end for j...
  }//end for m...

  return(1);
}//end CODE.

int CDWTCODEC::DECODE(void *Img)
{
  CImage *I;
  I = (CImage *)Img;

	if(I->m_ExtraColorType == BMP_INFO_SPECIFIES)
		{
    	error = 3;//ONLY YUV IMAGES MAY BE DECODED.
  	  return(0);
		}//end if m_ExtraColorType...

  //Determine the YUV data sizes.
	int YLine,YRow,UVLine,UVRow;
	YLine = I->GetYWidth();
	YRow = I->GetYHeight();
	UVLine = I->GetUVWidth();
	UVRow = I->GetUVHeight();
  //Do circular deconvolution along rows and columns.
  int R;
  int C;
  int i,j,m;
  int *Data;

  //Set the filter parameters.
  SetFilter(CurrDwtParam.WaveletType);

  for(m = (CurrDwtParam.LumLevels - 1); m >= 0 ; m--)
  {
    //Luminance.
    R = YRow >> m;
    C = YLine >> m;
    //In y direction.
    for(j = 0; j < C; j++)
    {
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_Y) + i*YLine + j);
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,
           (int *)(&(Data[0])),(int *)(&(Data[R/2])),R,Wk);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_Y) + i*YLine + j) = Wk[i];
    }//end for j...
  
    //In x direction.
    for(i = 0; i < R; i++)
    {
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,(int *)((I->m_Y) + i*YLine),
           (int *)((I->m_Y) + i*YLine + C/2),C,Wk);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_Y) + i*YLine + j) = Wk[j];
    }//end for i...
  }//end for m...
  
  for(m = (CurrDwtParam.ChrLevels - 1); m >= 0 ; m--)
  {
    //Do U colour component.
    R = UVRow >> m;
    C = UVLine >> m;
    //In y direction.
    for(j = 0; j < C; j++)
    {
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_U) + i*UVLine + j);
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,(int *)(&(Data[0])),
           (int *)(&(Data[R/2])),R,Wk);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_U) + i*UVLine + j) = Wk[i];
    }//end for j...
  
    //In x direction.
    for(i = 0; i < R; i++)
    {
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,(int *)((I->m_U) + i*UVLine),
           (int *)((I->m_U) + i*UVLine + C/2),C,Wk);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_U) + i*UVLine + j) = Wk[j];
    }//end for i...
  
    //Do V colour component.
    //In y direction.
    for(j = 0; j < C; j++)
    {
      //Extract the column into linear mem. Use half
      //of work mem.
      Data = (int *)(Wk + R);
      for(i = 0; i < R; i++)
        Data[i] = *((I->m_V) + i*UVLine + j);
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,(int *)(&(Data[0])),
           (int *)(&(Data[R/2])),R,Wk);
      //Copy to in-place.
      for(i = 0; i < R; i++)
        *((I->m_V) + i*UVLine + j) = Wk[i];
    }//end for j...
  
    //In x direction.
    for(i = 0; i < R; i++)
    {
      //Do Inverse transform.
      IDwt(CurrDwtParam.WaveletType,(int *)((I->m_V) + i*UVLine),
           (int *)((I->m_V) + i*UVLine + C/2),C,Wk);
      //Copy to in-place.
      for(j = 0; j < C; j++)
        *((I->m_V) + i*UVLine + j) = Wk[j];
    }//end for i...
  }//end for m...

  return(1);
}//end DECODE.

void CDWTCODEC::CloseCODEC(void)
{
  //Free the working memory.
	if(hWk)
	{
		GlobalUnlock(hWk);
 		GlobalFree(hWk);
 		hWk = NULL;
  	Wk = NULL;
	}//end if hWk...

}//end CloseCODEC.

const char *CDWTCODEC::GetErrorStr(int ErrorNum, char *ErrStr)
{
	static const char *ErrorStr[] = 
	  {"NO ERROR",													//0
		 "ONLY YUV IMAGES MAY BE CODED",      //1
		 "WORKING MEMORY UNAVAILABLE", 				//2
		 "ONLY YUV IMAGES MAY BE DECODED",		//3
		 "REFERENCE IMAGE NOT SETUP"};        //4

	strcpy(ErrStr,ErrorStr[ErrorNum]);
	return(ErrorStr[ErrorNum]);
}//end GetErrorStr.

////////////////////////////////////////////////////////
// Protected utility functions.
////////////////////////////////////////////////////////

void CDWTCODEC::SetFilter(int WaveType)
{
  //Set the filter parameters.
  switch(WaveType)
  {
    case STD_FILTER_6_10:
      F = SYMMETRIC_6_10;
      break;
    case STD_FILTER_18_18:
      F = SYMMETRIC_18;
      break;
    case STD_FILTER_24_20:
      F = SYMMETRIC_24_20;
      break;
    case STD_FILTER_5_3:
      F = ASYMMETRIC_5_3;
      break;
    case STD_FILTER_9_7:
      F = SPLINE_II;
      break;
    case STD_FILTER_13_11:
      F = ASYMMETRIC_13_11;
      break;
    case FAST_FILTER_9_7:
      Ff = LIFTING_9_7;
      break;
    case FAST_FILTER_5_3:
      Ff = LIFTING_5_3;
      break;
    case FAST_FILTER_13_11:
      Ff = LIFTING_13_11;
      break;
    case NN_6HID_FILTER_9_7:
      Fn = NN_6HID_9_7;
      break;
    case NN_8HID_FILTER_13_11:
      Fn = NN_8HID_13_11;
      break;
    case NN_16HID_FILTER_9_7:
      Fn = NN_16HID_9_7;
      break;
  }//end switch WaveType...
}//end SetFilter.

void CDWTCODEC::Dwt(int WaveType,int *x,int Len,int *y_Low,int *y_High)
{
  switch(WaveType)
  {
    case STD_FILTER_24_20:
    case STD_FILTER_18_18:
    case STD_FILTER_6_10:
    case STD_FILTER_5_3:
    case STD_FILTER_9_7:
    case STD_FILTER_13_11:
      DWT_1D(x,Len,y_Low,y_High);
      break;
    case FAST_LINEAR:
      FAST_DWT_1D_LINEAR(x,Len,y_Low,y_High);
      break;
    case FAST_CUBIC_4:
      FAST_DWT_1D_CUBIC4(x,Len,y_Low,y_High);
      break;
    case FAST_FILTER_5_3:
    case FAST_FILTER_9_7:
    case FAST_FILTER_13_11:
      FAST_DWT_1D(x,Len,y_Low,y_High);
      break;
    case NN_6HID_FILTER_9_7:
    case NN_8HID_FILTER_13_11:
    case NN_16HID_FILTER_9_7:
      NN_DWT_1D(x,Len,y_Low,y_High);
      break;
  }//end switch WaveType...
}//end Dwt.

void CDWTCODEC::IDwt(int WaveType,int *x_Low,int *x_High,int Len,int *Recon)
{
  switch(WaveType)
  {
    case STD_FILTER_24_20:
    case STD_FILTER_18_18:
    case STD_FILTER_6_10:
    case STD_FILTER_5_3:
    case STD_FILTER_9_7:
    case STD_FILTER_13_11:
      IDWT_1D(x_Low,x_High,Len,Recon);
      break;
    case FAST_LINEAR:
      FAST_IDWT_1D_LINEAR(x_Low,x_High,Len,Recon);
      break;
    case FAST_CUBIC_4:
      FAST_IDWT_1D_CUBIC4(x_Low,x_High,Len,Recon);
      break;
    case FAST_FILTER_5_3:
    case FAST_FILTER_9_7:
    case FAST_FILTER_13_11:
      FAST_IDWT_1D(x_Low,x_High,Len,Recon);
      break;
    case NN_6HID_FILTER_9_7:
    case NN_8HID_FILTER_13_11:
    case NN_16HID_FILTER_9_7:
      NN_IDWT_1D(x_Low,x_High,Len,Recon);
      break;
  }//end switch WaveType...
}//end IDwt.

/////////////////////////////////////////////////////////////////
// Forward discrete wavelet transform in 1-dimension with
// down sampling.
// Input: Signal of length Len.
// Output: Low and High pass signals of length Len/2 each.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::DWT_1D(int *x,int Len,int *y_Low,int *y_High)
{
  int j,k;
  int *X;
  unsigned long int XlL,XrL;
  double acc;

  //Set left and right data boundry limits.
  XlL = (unsigned long int)(x);
  XrL = (unsigned long int)(x + Len);
  for(j = 0; j < Len; j+=2)
  {
    X = (int *)(&(x[j]));
    //Low pass.
    acc = 0.0;
    for(k = -(F.h_off); k < (F.Num_h - F.h_off); k++)
    {
      unsigned long int P = (unsigned long int)((int *)(X+k));
      if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
      {
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(XlL + (XlL-P))));
      else if( P >= XrL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
      }//enf if CurrConvType...
      else if(CurrDwtParam.ConvType == CIRCULAR)
      {
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(X+Len+k)));
      else if( P >= XrL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(X-Len+k)));
      }//end else if ConvType... 
      else
      {
      //Boundary repeat convolution.
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)XlL));
      else if( P >= XrL ) 
        acc += F.ph[F.h_off + k] * (double)(*((int *)(XrL) - 1));
      }//end else...
    }//end for k...
    *y_Low++ = (int)(floor(acc + 0.5));
    //High pass.
    X = (int *)(&(x[j]));
    acc = 0.0;
    for(k = -(F.g_off); k < (F.Num_g - F.g_off); k++)
    {
      unsigned long int P = (unsigned long int)((int *)(X+k));
      if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
      {
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(XlL + (XlL-P))));
      else if( P >= XrL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
      }//end if CurrConvType...
      else if(CurrDwtParam.ConvType == CIRCULAR)
      {
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(X+Len+k)));
      else if( P >= XrL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(X-Len+k)));
      }//end else if CurrConvType...
      else
      {
      //Boundary repeat convolution.
      if( (P >= XlL) && (P < XrL) ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(X+k)));
      else if( P < XlL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)XlL));
      else if( P >= XrL ) 
        acc += F.pg[F.g_off + k] * (double)(*((int *)(XrL) - 1));
      }//end else...
    }//end for k...
    *y_High++ = (int)(floor(acc + 0.5));
  }//end for j...
}//end DWT_1D.

/////////////////////////////////////////////////////////////////
// Inverse discrete wavelet transform in 1-dimension with
// up sampling.
// Input: Low and High pass signals of length Len.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon)
{
  int j,k;
  int *X;
  unsigned long int XlL,XrL;
  double acc,t;

  for(j = 0; j < Len; j++)
  {
    //Low pass.
    XlL = (unsigned long int)(x_Low);
    XrL = (unsigned long int)(x_Low + (Len/2));
    X = (int *)(&(x_Low[j/2]));
    acc = 0.0;
    if( (j%2) == 0 ) //Do Even.
    {
      for(k = -(F.inv_h_even_off); k < (F.Num_inv_h_even - F.inv_h_even_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end if j...
    else //Do Odd.
    {
      for(k = -(F.inv_h_odd_off); k < (F.Num_inv_h_odd - F.inv_h_odd_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 1));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end else...
    t = acc;
    //High pass.
    XlL = (unsigned long int)(x_High);
    XrL = (unsigned long int)(x_High + (Len/2));
    X = (int *)(&(x_High[j/2]));
    acc = 0.0;
    if( (j%2) == 0 ) //Do Even.
    {
      for(k = -(F.inv_g_even_off); k < (F.Num_inv_g_even - F.inv_g_even_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end if j...
    else //Do Odd.
    {
      for(k = -(F.inv_g_odd_off); k < (F.Num_inv_g_odd - F.inv_g_odd_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 1));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end else...
    //Add.
    Recon[j] = (int)(floor((t + acc)*2.0 + 0.5));
  }//end for j...
}//end IDWT_1D.

/////////////////////////////////////////////////////////////////
// Inverse discrete wavelet transform in 1-dimension with
// up sampling.
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len. (The reconstructed
//         Low and High pass components of length Len).
/////////////////////////////////////////////////////////////////
void CDWTCODEC::IDWT_1D(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon)
{
  int j,k;
  int *X;
  unsigned long int XlL,XrL;
  double acc;

  for(j = 0; j < Len; j++)
  {
    //Low pass.
    XlL = (unsigned long int)(x_Low);
    XrL = (unsigned long int)(x_Low + (Len/2));
    X = (int *)(&(x_Low[j/2]));
    acc = 0.0;
    if( (j%2) == 0 ) //Do Even.
    {
      for(k = -(F.inv_h_even_off); k < (F.Num_inv_h_even - F.inv_h_even_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_h_even[F.inv_h_even_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end if j...
    else //Do Odd.
    {
      for(k = -(F.inv_h_odd_off); k < (F.Num_inv_h_odd - F.inv_h_odd_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 1));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_h_odd[F.inv_h_odd_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end else...
    y_Low[j] = (int)(floor((acc * 2.0) + 0.5));
    //High pass.
    XlL = (unsigned long int)(x_High);
    XrL = (unsigned long int)(x_High + (Len/2));
    X = (int *)(&(x_High[j/2]));
    acc = 0.0;
    if( (j%2) == 0 ) //Do Even.
    {
      for(k = -(F.inv_g_even_off); k < (F.Num_inv_g_even - F.inv_g_even_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 2));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_g_even[F.inv_g_even_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end if j...
    else //Do Odd.
    {
      for(k = -(F.inv_g_odd_off); k < (F.Num_inv_g_odd - F.inv_g_odd_off); k++)
      {
        unsigned long int P = (unsigned long int)((int *)(X+k));
        if(CurrDwtParam.ConvType == BOUNDARY_REFLECTION)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XlL+(XlL-P))));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL-(P-XrL)) - 1));
        }//end if ConvType...
        else if(CurrDwtParam.ConvType == CIRCULAR)
        {
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL-(XrL-P)) - 1));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XlL+(P-XrL))));
        }//end else if ConvType...
        else
        {
        //Boundary repeat Deconvolution.
        if( (P >= XlL) && (P < XrL) ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(X+k)));
        else if( P < XlL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)XlL));
        else if( P >= XrL ) 
          acc += F.pinv_g_odd[F.inv_g_odd_off + k] * (double)(*((int *)(XrL) - 1));
        }//end else...
      }//end for k...
    }//end else...
    y_High[j] = (int)(floor((acc * 2.0) + 0.5));
    //Add.
    Recon[j] = y_Low[j] + y_High[j];
  }//end for j...
}//end IDWT_1D.

/////////////////////////////////////////////////////////////////
// Fast discrete wavelet transform in 1-dimension with
// down sampling by 2.
// Input: Signal of length Len.
// Output: Low and High pass signals of length Len/2.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_DWT_1D_CUBIC4(int *x,int Len,int *y_Low,int *y_High)
{
  int j;
  int y;
  double acc;

  int tx[4];
  double l = 0.0;
  double r = 0.0;

  for(j = 0; j < Len; j+=2)
  {
    //1st approx. is even sub sampled values.
    y_Low[j/2] = x[j];

    //Interpolate the odd coeff with cubic spline.
    if( j == 0 )
      {
        tx[0] = x[j+2];
        tx[1] = x[j];
        tx[2] = x[j+2];
        tx[3] = x[j+4];
      }//end if j...
    else if( j == (Len - 4) )
      {
        tx[0] = x[j-2];
        tx[1] = x[j];
        tx[2] = x[j+2];
        tx[3] = x[j+2];
      }//end else if j...
    else if( j == (Len - 2) )
      {
        tx[0] = x[j-2];
        tx[1] = x[j];
        tx[2] = x[j];
        tx[3] = x[j-2];
      }//end else if j...
    else
      {
        tx[0] = x[j-2];
        tx[1] = x[j];
        tx[2] = x[j+2];
        tx[3] = x[j+4];
      }//end else...
    r = 0.0; // always reset right 2nd deriv.
    y = CubicSpline4(tx,&l,&r);

    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[j/2] = x[j+1] - y;

  }//end for j...

  //Update 1st approx. coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(y_High[j])) + (0.25 * (double)(y_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(y_High[j]));
    y_Low[j] = y_Low[j] + (int)(acc + 0.5);
  }//end for j...

}//end FAST_DWT_1D_CUBIC4.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len. (The reconstructed
//         Low and High pass components of length Len).
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_IDWT_1D_CUBIC4(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon)
{
  int j;
  double acc;
  int y;

  int tx[4];
  double l = 0.0;
  double r = 0.0;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(x_High[j])) + (0.25 * (double)(x_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(x_High[j]));
    y_Low[2*j] = x_Low[j] - (int)(acc + 0.5);
    y_Low[2*j + 1] = 0;
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate the odd coeff with cubic spline.
    if( j == 0 )
      {
        tx[0] = y_Low[j+2];
        tx[1] = y_Low[j];
        tx[2] = y_Low[j+2];
        tx[3] = y_Low[j+4];
      }//end if j...
    else if( j == (Len - 4) )
      {
        tx[0] = y_Low[j-2];
        tx[1] = y_Low[j];
        tx[2] = y_Low[j+2];
        tx[3] = y_Low[j+2];
      }//end else if j...
    else if( j == (Len - 2) )
      {
        tx[0] = y_Low[j-2];
        tx[1] = y_Low[j];
        tx[2] = y_Low[j];
        tx[3] = y_Low[j-2];
      }//end else if j...
    else
      {
        tx[0] = y_Low[j-2];
        tx[1] = y_Low[j];
        tx[2] = y_Low[j+2];
        tx[3] = y_Low[j+4];
      }//end else...
    r = 0.0; // always reset right 2nd deriv.
    y = CubicSpline4(tx,&l,&r);

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    y_High[j + 1] = x_High[j/2] + y;
    y_High[j] = 0;

    //Copy into place.
    Recon[j] = y_Low[j];
    Recon[j+1] = y_High[j+1];

  }//end for j...

}//end FAST_IDWT_1D_CUBIC4.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_IDWT_1D_CUBIC4(int *x_Low,int *x_High,int Len,int *Recon)
{
  int j;
  double acc;
  int y;

  int tx[4];
  double l = 0.0;
  double r = 0.0;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(x_High[j])) + (0.25 * (double)(x_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(x_High[j]));
    Recon[2*j] = x_Low[j] - (int)(acc + 0.5);
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate the odd coeff with cubic spline.
    if( j == 0 )
      {
        tx[0] = Recon[j+2];
        tx[1] = Recon[j];
        tx[2] = Recon[j+2];
        tx[3] = Recon[j+4];
      }//end if j...
    else if( j == (Len - 4) )
      {
        tx[0] = Recon[j-2];
        tx[1] = Recon[j];
        tx[2] = Recon[j+2];
        tx[3] = Recon[j+2];
      }//end else if j...
    else if( j == (Len - 2) )
      {
        tx[0] = Recon[j-2];
        tx[1] = Recon[j];
        tx[2] = Recon[j];
        tx[3] = Recon[j-2];
      }//end else if j...
    else
      {
        tx[0] = Recon[j-2];
        tx[1] = Recon[j];
        tx[2] = Recon[j+2];
        tx[3] = Recon[j+4];
      }//end else...
    r = 0.0; // always reset right 2nd deriv.
    y = CubicSpline4(tx,&l,&r);

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    Recon[j + 1] = x_High[j/2] + y;

  }//end for j...

}//end FAST_IDWT_1D_CUBIC4.

/////////////////////////////////////////////////////////////////
// Interpolate a value half way between the 2nd and 3rd values
// of the input array of 4 values. 
// Input: x_4[4] data points, boundry left and right 2nd deriv.
// Output: Interpolated value and left and right 2nd deriv.
//         around the point.
/////////////////////////////////////////////////////////////////
int CDWTCODEC::CubicSpline4(int *x_4,double *left,double *right)
{
  double a = (double)(x_4[2] - x_4[0]) * 2.0;
  double b = (double)(x_4[3] - x_4[1]) * 2.0;

  double t_right = (b * 2.0) - a + (*left - (*right * 2.0))/3.0;
  double t_left = (a * 2.0) - b + (*right - (*left * 2.0))/3.0;
  *left = t_left;
  *right = t_right;
  double y = (double)(x_4[1] + x_4[2])/2.0 - (t_left + t_right)*0.0625;
  return((int)(y + 0.5));
}//end CubicSpline4.

/////////////////////////////////////////////////////////////////
// Fast discrete wavelet transform in 1-dimension with
// down sampling by 2.
// Input: Signal of length Len.
// Output: Low and High pass signals of length Len/2.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_DWT_1D_LINEAR(int *x,int Len,int *y_Low,int *y_High)
{
  int j;
  double acc;
  int y;

  for(j = 0; j < Len; j+=2)
  {
    //1st approx. is even sub sampled values.
    y_Low[j/2] = x[j];

    //Interpolate the odd coeff as average.
    if( j < (Len - 2) )
      acc = ((double)(x[j]) + (double)(x[j+2])) * 0.5;
    else
      acc = ((double)(x[j]));
    y = (int)(acc + 0.5);

    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[j/2] = x[j+1] - y;

  }//end for j...

  //Update 1st approx. coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(y_High[j])) + (0.25 * (double)(y_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(y_High[j]));
    y_Low[j] = y_Low[j] + (int)(acc + 0.5);
  }//end for j...

}//end FAST_DWT_1D_LINEAR.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len. (The reconstructed
//         Low and High pass components of length Len).
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_IDWT_1D_LINEAR(int *x_Low,int *x_High,int Len,int *y_Low,int *y_High,int *Recon)
{
  int j;
  double acc;
  int y;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(x_High[j])) + (0.25 * (double)(x_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(x_High[j]));
    y_Low[2*j] = x_Low[j] - (int)(acc + 0.5);
    y_Low[2*j + 1] = 0;
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate an approx. of the odd coeff using low pass filter.
    if( j < (Len - 2) )
      acc = ((double)(y_Low[j]) + (double)(y_Low[j+2])) * 0.5;
    else
      acc = ((double)(y_Low[j]));
    y = (int)(acc + 0.5);

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    y_High[j + 1] = x_High[j/2] + y;
    y_High[j] = 0;

    //Copy into place.
    Recon[j] = y_Low[j];
    Recon[j+1] = y_High[j+1];

  }//end for j...

}//end FAST_IDWT_1D_LINEAR.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_IDWT_1D_LINEAR(int *x_Low,int *x_High,int Len,int *Recon)
{
  int j;
  double acc;
  int y;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < (Len/2); j++)
  {
    if( j != 0 )
      acc = (0.25 * (double)(x_High[j])) + (0.25 * (double)(x_High[j-1]));
    else //Left boundry.
      acc = (0.5 * (double)(x_High[j]));
    Recon[2*j] = x_Low[j] - (int)(acc + 0.5);
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate an approx. of the odd coeff using low pass filter.
    if( j < (Len - 2) )
      acc = ((double)(Recon[j]) + (double)(Recon[j+2])) * 0.5;
    else
      acc = ((double)(Recon[j]));
    y = (int)(acc + 0.5);

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    Recon[j + 1] = x_High[j/2] + y;

  }//end for j...

}//end FAST_IDWT_1D_LINEAR.

/////////////////////////////////////////////////////////////////
// Fast discrete wavelet transform in 1-dimension with
// down sampling by 2.
// Input: Signal of length Len.
// Output: Low and High pass signals of length Len/2.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_DWT_1D(int *x,int Len,int *y_Low,int *y_High)
{
  int i,j;
  int SubLen = Len/2;

  double *wx = new double[Ff.PredLen];
  double *vx = new double[Ff.UpdateLen];
  double r;
  int start;

  for(j = 0; j < Len; j+=2)
  {
    //1st approx. is even sub sampled values.
    y_Low[j/2] = x[j];
  }//end for j...

  for(j = 0; j < SubLen; j++)
  {
    //Interpolate the odd coeff with learned w coeff 
    //from the even coefficients.
    r = 0.0;
    start = j - (Ff.PredLen/2 - 1);
    for(i = 0; i < Ff.PredLen; i++)
    {
      if((start+i) < 0)
        wx[i] = (double)(y_Low[-(start+i)]);
      else if((start+i) >= SubLen)
        wx[i] = (double)(y_Low[SubLen-((start+i)-(SubLen-1))]);
      else
        wx[i] = (double)(y_Low[start+i]);
      r += Ff.pPred[i]*wx[i];
    }//end for i...

    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[j] = (int)(floor(r - 0.5*(double)x[2*j+1] + 0.5));

  }//end for j...

  //Update 1st approx. coeff with piece of wavelet.
  for(j = 0; j < SubLen; j++)
  {
    r = 0.0;
    start = j - (Ff.UpdateLen/2);
    for(i = 0; i < Ff.UpdateLen; i++)
    {
      if((start+i) < 0)
        vx[i] = (double)(y_High[-(start+i)]);
      else if((start+i) >= SubLen)
        vx[i] = (double)(y_High[SubLen-((start+i)-(SubLen-1))]);
      else
        vx[i] = (double)(y_High[start+i]);
      r += Ff.pUpdate[i]*vx[i];
    }//end for i...

    y_Low[j] = y_Low[j] + (int)(floor(r + 0.5));
  }//end for j...
  delete(wx);
  delete(vx);
}//end FAST_DWT_1D.

/////////////////////////////////////////////////////////////////
// Fast inverse discrete wavelet transform in 1-dimension with
// up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::FAST_IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon)
{
  int i,j;
  int SubLen = Len/2;

  double *wx = new double[Ff.PredLen];
  double *vx = new double[Ff.UpdateLen];
  double r;
  int start;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < SubLen; j++)
  {
    r = 0.0;
    start = j - (Ff.UpdateLen/2);
    for(i = 0; i < Ff.UpdateLen; i++)
    {
      if((start+i) < 0)
        vx[i] = (double)(x_High[-(start+i)]);
      else if((start+i) >= SubLen)
        vx[i] = (double)(x_High[SubLen-((start+i)-(SubLen-1))]);
      else
        vx[i] = (double)(x_High[start+i]);
      r += Ff.pUpdate[i]*vx[i];
    }//end for i...

    Recon[2*j] = x_Low[j] - (int)(floor(r + 0.5));
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate the odd coeff with w and the new even coeff.
    r = 0.0;
    start = (j/2) - (Ff.PredLen/2 - 1);
    for(i = 0; i < Ff.PredLen; i++)
    {
      if((start+i) < 0)
        wx[i] = (double)(Recon[2*(-(start+i))]);
      else if((start+i) >= SubLen)
        wx[i] = (double)(Recon[2*(SubLen-((start+i)-(SubLen-1)))]);
      else
        wx[i] = (double)(Recon[2*(start+i)]);
      r += Ff.pPred[i]*wx[i];
    }//end for i...

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    Recon[j + 1] = (int)(floor(2.0*(r - (double)x_High[j/2]) + 0.5));

  }//end for j...
  delete(wx);
  delete(vx);
}//end FAST_IDWT_1D.

/////////////////////////////////////////////////////////////////
// Neural network based discrete wavelet transform in 1-dimension 
// with down sampling by 2.
// Input: Signal of length Len.
// Output: Low and High pass signals of length Len/2.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::NN_DWT_1D(int *x,int Len,int *y_Low,int *y_High)
{
  int i,j,k;
  int SubLen = Len/2;

  double *wx = new double[Fn.PredLen];
  double *hid = new double[Fn.HidLay];
  double *vx = new double[Fn.UpdateLen];
  double r;
  int start;

  for(j = 0; j < Len; j+=2)
  {
    //1st approx. is even sub sampled values.
    y_Low[j/2] = x[j];
  }//end for j...

  for(j = 0; j < SubLen; j++)
  {
    //Interpolate the odd coeff with learned w coeff 
    //from the even coefficients.

    // Extract the coeff. for interpolation.
    start = j - (Fn.PredLen/2 - 1);
    for(i = 0; i < Fn.PredLen; i++)
    {
      if((start+i) < 0)
        wx[i] = (double)(y_Low[-(start+i)]);
      else if((start+i) >= SubLen)
        wx[i] = (double)(y_Low[SubLen-((start+i)-(SubLen-1))]);
      else
        wx[i] = (double)(y_Low[start+i]);
    }//end for i...
    // Apply the neural network as an interpolator.
    r = 0.0;
    for(k = 0; k < Fn.HidLay; k++)
    {
      hid[k] = 0.0;
      for(i = 0; i < Fn.PredLen; i++)
      {
        hid[k] += Fn.pPred[(k*Fn.PredLen)+i]*wx[i];
      }//end for i...
      // Calc. non-linearity.
      hid[k] = 255.0*tanh(0.25*hid[k]/255.0);
      // Calc. linear output.
      r += Fn.pLin[k]*hid[k];
    }//end for k...

    //Wavelet coeff is diff. between odd and interpolated values.
    y_High[j] = (int)(floor(r - 0.5*(double)x[2*j+1] + 0.5));
//    y_High[j] = (int)(floor((r - 0.5*(double)x[2*j+1])/R2 + 0.5));

  }//end for j...

  //Update 1st approx. coeff with piece of wavelet.
  for(j = 0; j < SubLen; j++)
  {
    r = 0.0;
    start = j - (Fn.UpdateLen/2);
    for(i = 0; i < Fn.UpdateLen; i++)
    {
      if((start+i) < 0)
        vx[i] = (double)(y_High[-(start+i)]);
      else if((start+i) >= SubLen)
        vx[i] = (double)(y_High[SubLen-((start+i)-(SubLen-1))]);
      else
        vx[i] = (double)(y_High[start+i]);
      r += Ff.pUpdate[i]*vx[i];
    }//end for i...
// For non-linear output stage:    r = 255.0 * tanh(0.25*r/255.0);

    y_Low[j] = y_Low[j] + (int)(floor(r + 0.5));
//    y_Low[j] = (int)((double)(y_Low[j] + (int)(floor(r + 0.5))) / R2);
  }//end for j...
  delete(wx);
  delete(hid);
  delete(vx);
}//end NN_DWT_1D.

/////////////////////////////////////////////////////////////////
// Neural network based inverse discrete wavelet transform in 
// 1-dimension with up sampling by 2. 
// Input: Low and High pass signals of length Len/2.
// Output: Reconstructed signal of length Len.
/////////////////////////////////////////////////////////////////
void CDWTCODEC::NN_IDWT_1D(int *x_Low,int *x_High,int Len,int *Recon)
{
  int i,j,k;
  int SubLen = Len/2;

  double *wx = new double[Fn.PredLen];
  double *hid = new double[Fn.HidLay];
  double *vx = new double[Fn.UpdateLen];
  double r;
  int start;

  //Create even coeff with piece of wavelet.
  for(j = 0; j < SubLen; j++)
  {
    r = 0.0;
    start = j - (Fn.UpdateLen/2);
    for(i = 0; i < Fn.UpdateLen; i++)
    {
      if((start+i) < 0)
        vx[i] = (double)(x_High[-(start+i)]);
      else if((start+i) >= SubLen)
        vx[i] = (double)(x_High[SubLen-((start+i)-(SubLen-1))]);
      else
        vx[i] = (double)(x_High[start+i]);
      r += Fn.pUpdate[i]*vx[i];
    }//end for i...
// For non-linear output stage:    r = 255.0 * tanh(0.25*r/255.0);

    Recon[2*j] = x_Low[j] - (int)(floor(r + 0.5));
//    Recon[2*j] = (int)((double)x_Low[j] * R2) - (int)(floor(r + 0.5));
  }//end for j...

  for(j = 0; j < Len; j+=2)
  {
    //Interpolate the odd coeff with w and the new even coeff.

    // Extract the coeff. for interpolation.
    start = (j/2) - (Fn.PredLen/2 - 1);
    for(i = 0; i < Fn.PredLen; i++)
    {
      if((start+i) < 0)
        wx[i] = (double)(Recon[2*(-(start+i))]);
      else if((start+i) >= SubLen)
        wx[i] = (double)(Recon[2*(SubLen-((start+i)-(SubLen-1)))]);
      else
        wx[i] = (double)(Recon[2*(start+i)]);
    }//end for i...
    // Apply the neural network as an interpolator.
    r = 0.0;
    for(k = 0; k < Fn.HidLay; k++)
    {
      hid[k] = 0.0;
      for(i = 0; i < Fn.PredLen; i++)
      {
        hid[k] += Fn.pPred[(k*Fn.PredLen)+i]*wx[i];
      }//end for i...
      // Calc. non-linearity.
      hid[k] = 255.0*tanh(0.25*hid[k]/255.0);
      // Calc. linear output.
      r += Fn.pLin[k]*hid[k];
    }//end for k...

    //Create odd coeff with sum of wavelet coeff and interpolated values.
    Recon[j + 1] = (int)(floor(2.0*(r - (double)x_High[j/2]) + 0.5));
//    Recon[j + 1] = (int)(floor(2.0*(r - ((double)x_High[j/2] * R2)) + 0.5));

  }//end for j...
  delete(wx);
  delete(hid);
  delete(vx);
}//end NN_IDWT_1D.


