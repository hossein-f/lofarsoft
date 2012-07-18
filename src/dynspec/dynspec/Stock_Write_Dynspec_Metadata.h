#ifndef DEF_STOCK_WRITE_DYNSPEC_METADATA
#define DEF_STOCK_WRITE_DYNSPEC_METADATA

#include<string>
#include<iostream>
#include<vector>

#include <dal/lofar/BF_File.h>

using namespace DAL;

class Stock_Write_Dynspec_Metadata
{  
  // Public Methods
  public:
  
    Stock_Write_Dynspec_Metadata();
    ~Stock_Write_Dynspec_Metadata();


    void stockDynspecMetadata(std::string attr_69,std::string attr_70,std::string attr_71,std::string attr_72,std::string attr_73,std::string attr_74,std::string attr_75,double attr_76,double attr_77,std::string attr_78,
			      std::string attr_79,std::string attr_80,double attr_81,double attr_82,double attr_83,double attr_84,double attr_85,double attr_86,double attr_87,double attr_88,double attr_89,
			      std::string attr_90,double attr_91,std::vector<std::string> attr_92,std::string attr_93,double attr_94,std::string attr_95,bool attr_96,std::vector<std::string> attr_97,bool attr_98,
			      std::string attr_99,std::string attr_100,std::vector<double> attr_101,std::vector<std::string> attr_102,std::string attr_103,double attr_104,std::string attr_105,std::string attr_106,
			      double attr_107,double attr_108,std::vector<std::string> attr_109,std::string attr_110,std::string attr_111,std::vector<std::string> attr_112,int attr_113,std::vector<std::string> attr_114,
			      std::vector<std::string> attr_115,std::vector<double> attr_116,std::vector<double> attr_117,std::vector<double> attr_118,std::vector<double> attr_119,std::vector<double> attr_121,
			      std::vector<double> attr_122,std::string attr_123,std::string attr_124,std::vector<std::string> attr_125,int attr_126,std::vector<std::string> attr_127,std::vector<std::string> attr_128,
			      std::vector<double> attr_129,std::vector<double> attr_130,std::vector<double> attr_131,std::vector<double> attr_132,std::vector<double> attr_133,std::vector<double> attr_135,
			      std::string attr_137,std::string attr_138,std::vector<std::string> attr_139,int attr_140,std::vector<std::string> attr_141,std::vector<std::string> attr_142,int Ntime,int Nspectral,
			      std::string attr_147,std::string attr_148,int attr_149,std::string attr_150,std::string attr_151,int attr_152,std::vector<std::string> attr_153,std::vector<std::string> attr_154,
			      std::vector<std::string> attr_155,std::vector<std::string> attr_156,std::vector<std::string> attr_157,std::vector<std::string> attr_158,std::string attr_159,bool attr_160,bool attr_161,
			      bool attr_162,bool attr_163);
		 
			 
    void writeDynspecMetadata(Group &dynspec_grp,std::string pathDir,std::string obsName,std::string pathFile,std::string outputFile,File &root_grp,int i,int j,int k,int l,int q,int obsNofStockes,std::vector<std::string> stokesComponent,
			      float memoryRAM);

  // Private Attributes
  private:

    std::string m_attr_69;
    std::string m_attr_70;
    std::string m_attr_71;
    std::string m_attr_72;
    std::string m_attr_73;
    std::string m_attr_74;
    std::string m_attr_75;
    double m_attr_76;
    double m_attr_77;
    std::string m_attr_78;
    std::string m_attr_79;
    std::string m_attr_80;
    double m_attr_81;
    double m_attr_82;
    double m_attr_83;
    double m_attr_84;
    double m_attr_85;
    double m_attr_86;
    double m_attr_87;
    double m_attr_88;
    double m_attr_89;
    std::string m_attr_90;
    double m_attr_91;
 
    int m_size_attr_92;
    std::vector<std::string> m_attr_92;
 
    std::string m_attr_93;
    double m_attr_94;
    std::string m_attr_95;
    bool m_attr_96;
 
    int m_size_attr_97;
    std::vector<std::string> m_attr_97;
 
    bool m_attr_98;
    std::string m_attr_99; 
    std::string m_attr_100;   
    
    int m_size_attr_101;
    std::vector<double> m_attr_101;
    
    int m_size_attr_102;    
    std::vector<std::string> m_attr_102;
    
    std::string m_attr_103;
    double m_attr_104;
    std::string m_attr_105;
    std::string m_attr_106;
    double m_attr_107;
    double m_attr_108;
    
    int m_size_attr_109;    
    std::vector<std::string> m_attr_109;   
   
    std::string m_attr_110;
    std::string m_attr_111;
    
    int m_size_attr_112;
    std::vector<std::string> m_attr_112;
    
    int m_attr_113;
    
    int m_size_attr_114;
    std::vector<std::string> m_attr_114;
    
    int m_size_attr_115;
    std::vector<std::string> m_attr_115;
    
    int m_size_attr_116;
    std::vector<double> m_attr_116;
    
    int m_size_attr_117;
    std::vector<double> m_attr_117;
    
    int m_size_attr_118;
    std::vector<double> m_attr_118;
    
    int m_size_attr_119;
    std::vector<double> m_attr_119;
    
    int m_size_attr_121;
    std::vector<double> m_attr_121;
			        
    int m_size_attr_122;
    std::vector<double> m_attr_122;   
   
    std::string m_attr_123;
    std::string m_attr_124;
    
    int m_size_attr_125;
    std::vector<std::string> m_attr_125;
    
    int m_attr_126;
    
    int m_size_attr_127;
    std::vector<std::string> m_attr_127;
    
    int m_size_attr_128;
    std::vector<std::string> m_attr_128;
    
    int m_size_attr_129;
    std::vector<double> m_attr_129;
    
    int m_size_attr_130;
    std::vector<double> m_attr_130;
    
    int m_size_attr_131;
    std::vector<double> m_attr_131;
    
    int m_size_attr_132;
    std::vector<double> m_attr_132;
    
    int m_size_attr_133;
    std::vector<double> m_attr_133;
    
    int m_size_attr_135;
    std::vector<double> m_attr_135;   
   
    std::string m_attr_137;
    std::string m_attr_138;
    
    int m_size_attr_139;    
    std::vector<std::string> m_attr_139;
    
    int m_attr_140;
    
    int m_size_attr_141;    
    std::vector<std::string> m_attr_141;
    
    int m_size_attr_142;   
    std::vector<std::string> m_attr_142;
    
    int m_Ntime;
    int m_Nspectral;

    std::string m_attr_147;
    std::string m_attr_148;
    int m_attr_149;
    std::string m_attr_150;
    std::string m_attr_151;
    int m_attr_152;
    
    int m_size_attr_153; 
    std::vector<std::string> m_attr_153;
    
    int m_size_attr_154; 
    std::vector<std::string> m_attr_154;
    
    int m_size_attr_155; 
    std::vector<std::string> m_attr_155;
    
    int m_size_attr_156; 
    std::vector<std::string> m_attr_156;
    
    int m_size_attr_157; 
    std::vector<std::string> m_attr_157;
    
    int m_size_attr_158; 
    std::vector<std::string> m_attr_158;    
    
    std::string m_attr_159;
    bool m_attr_160;
    bool m_attr_161;
    bool m_attr_162;
    bool m_attr_163;  
};
 
#endif
