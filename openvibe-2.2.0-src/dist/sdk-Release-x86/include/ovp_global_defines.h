#ifndef __OpenViBEPlugins_Global_Defines_H__
#define __OpenViBEPlugins_Global_Defines_H__


// -----------------------------------------------------
// XML Scenario importer
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_XMLScenarioImporter                                                                            OpenViBE::CIdentifier(0xe80c3ea2, 0x149c4a05)
#define OVP_GD_ClassId_Algorithm_XMLScenarioExporter                                                                            OpenViBE::CIdentifier(0x53693531, 0xb136cf3f)
#define OVP_GD_TypeId_ClassificationPairwiseStrategy OpenViBE::CIdentifier(0x0DD51C74, 0x3C4E74C9)
#define OVP_GD_TypeId_OneVsOne_DecisionAlgorithms OpenViBE::CIdentifier(0xDEC1510, 0xDEC1510)
#define OVP_GD_ClassId_Algorithm_PairwiseStrategy_PKPD OpenViBE::CIdentifier(0x26EF6DDA, 0xF137053C)
#define OVP_GD_ClassId_Algorithm_PairwiseDecision_Voting OpenViBE::CIdentifier(0xA111B830, 0x4679BAFD)
#define OVP_GD_ClassId_Algorithm_PairwiseDecision_HT OpenViBE::CIdentifier(0xD24F7F19, 0xA744FAD2)

// Algorithm OVMatrixFileReader

#define OVP_GD_ClassId_Algorithm_OVMatrixFileReader                               OpenViBE::CIdentifier(0x10661A33, 0x0B0F44A7)
#define OVP_GD_ClassId_Algorithm_OVMatrixFileReaderDesc                           OpenViBE::CIdentifier(0x0E873B5E, 0x0A287FCB)


#define OVP_GD_Algorithm_OVMatrixFileReader_InputParameterId_Filename             OpenViBE::CIdentifier(0x28F87B29, 0x0B09737E)
#define OVP_GD_Algorithm_OVMatrixFileReader_OutputParameterId_Matrix              OpenViBE::CIdentifier(0x2F9521E0, 0x027D789F)
#define OVP_GD_Algorithm_OVMatrixFileReader_InputTriggerId_Open                   OpenViBE::CIdentifier(0x2F996376, 0x2A942485)
#define OVP_GD_Algorithm_OVMatrixFileReader_InputTriggerId_Load                   OpenViBE::CIdentifier(0x22841807, 0x102D681C)
#define OVP_GD_Algorithm_OVMatrixFileReader_InputTriggerId_Close                  OpenViBE::CIdentifier(0x7FDE77DA, 0x384A0B3D)
#define OVP_GD_Algorithm_OVMatrixFileReader_OutputTriggerId_Error                 OpenViBE::CIdentifier(0x6D4F2F4B, 0x05EC6CB9)
#define OVP_GD_Algorithm_OVMatrixFileReader_OutputTriggerId_DataProduced          OpenViBE::CIdentifier(0x76F46051, 0x003B6FE8)

// -----------------------------------------------------
// EBML stream encoder
// -----------------------------------------------------
#define OVP_GD_ClassId_Algorithm_StreamStructureDecoder     OpenViBE::CIdentifier(0xA7EF3E8B, 0x4CF70B74)
#define OVP_GD_ClassId_Algorithm_StreamStructureDecoderDesc OpenViBE::CIdentifier(0x2E361099, 0xCBE828A7)

#define OVP_GD_Algorithm_StreamStructureDecoder_InputParameterId_MemoryBufferToDecode                              OpenViBE::CIdentifier(0x2F98EA3C, 0xFB0BE096)
#define OVP_GD_Algorithm_StreamStructureDecoder_OutputTriggerId_ReceivedHeader                                     OpenViBE::CIdentifier(0x815234BF, 0xAABAE5F2)
#define OVP_GD_Algorithm_StreamStructureDecoder_OutputTriggerId_ReceivedBuffer                                     OpenViBE::CIdentifier(0xAA2738BF, 0xF7FE9FC3)
#define OVP_GD_Algorithm_StreamStructureDecoder_OutputTriggerId_ReceivedEnd                                        OpenViBE::CIdentifier(0xC4AA114C, 0x628C2D77)

// -----------------------------------------------------
// Streamed matrix stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_StreamedMatrixStreamEncoder                                                                    OpenViBE::CIdentifier(0x5cb32c71, 0x576f00a6)
#define OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix                                                    OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303)
#define OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                      OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734)
#define OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer                                                OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a)
#define OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd                                                   OpenViBE::CIdentifier(0x3fc23508, 0x806753d8)
#define OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader                                                OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f)

// -----------------------------------------------------
// Experiment information stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ExperimentInformationStreamDecoder                                                             OpenViBE::CIdentifier(0x6fa7d52b, 0x80e2abd6)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode                               OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectName                                       OpenViBE::CIdentifier(0x3d3826ea, 0xe8883815)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_ExperimentIdentifier                              OpenViBE::CIdentifier(0x40259641, 0x478c73de)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_LaboratoryName                                    OpenViBE::CIdentifier(0x5ca80fa5, 0x774f01cb)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectGender                                     OpenViBE::CIdentifier(0x7d5059e8, 0xe4d8b38d)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectIdentifier                                 OpenViBE::CIdentifier(0x97c5d20d, 0x203e65b3)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_TechnicianName                                    OpenViBE::CIdentifier(0xb8a94b68, 0x389393d9)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_ExperimentDate                                    OpenViBE::CIdentifier(0xbc0266a2, 0x9c2935f1)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectAge                                        OpenViBE::CIdentifier(0xc36c6b08, 0x5227380a)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_TechnicianIdentifier                              OpenViBE::CIdentifier(0xc8ecfbbc, 0x0dcda310)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_LaboratoryIdentifier                              OpenViBE::CIdentifier(0xe761d3d4, 0x44ba1ebf)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader                                      OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer                                      OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3)
#define OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd                                         OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77)

// -----------------------------------------------------
// Channel localisation stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ChannelLocalisationStreamEncoder                                                               OpenViBE::CIdentifier(0xc4aa738a, 0x2368c0ea)
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputParameterId_Matrix                                               OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputParameterId_Dynamic                                              OpenViBE::CIdentifier(0xcf5dd4f8, 0xc2ff2878)
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                 OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputTriggerId_EncodeBuffer                                           OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputTriggerId_EncodeEnd                                              OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_ChannelLocalisationStreamEncoder_InputTriggerId_EncodeHeader                                           OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Signal stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_SignalStreamDecoder                                                                            OpenViBE::CIdentifier(0x7237c149, 0x0ca66da7)
#define OVP_GD_Algorithm_SignalStreamDecoder_InputParameterId_MemoryBufferToDecode                                              OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_SamplingRate                                                     OpenViBE::CIdentifier(0x363d8d79, 0xeefb912c)
#define OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix                                                           OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d)
#define OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedHeader                                                     OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedBuffer                                                     OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_SignalStreamDecoder_OutputTriggerId_ReceivedEnd                                                        OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Spectrum stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_SpectrumStreamEncoder                                                                          OpenViBE::CIdentifier(0xb3e252db, 0xc3214498)
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_Matrix                                                          OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_FrequencyAbscissa                                               OpenViBE::CIdentifier(0x05C91BD6, 0x2D8C4083)
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputParameterId_SamplingRate                                                    OpenViBE::CIdentifier(0x02D25E1B, 0x76A1019B)
#define OVP_GD_Algorithm_SpectrumStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                            OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputTriggerId_EncodeBuffer                                                      OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputTriggerId_EncodeEnd                                                         OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_SpectrumStreamEncoder_InputTriggerId_EncodeHeader                                                      OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Acquisition stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_AcquisitionStreamDecoder                                                                       OpenViBE::CIdentifier(0x1e0812b7, 0x3f686dd4)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_InputParameterId_MemoryBufferToDecode                                         OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_StimulationStream                                           OpenViBE::CIdentifier(0x08fc3c12, 0x86a07bf7)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ChannelUnitsStream                                          OpenViBE::CIdentifier(0x11b93981, 0x6e5da9b0)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_SignalStream                                                OpenViBE::CIdentifier(0x42c0d7bd, 0xbbcea3f3)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ChannelLocalisationStream                                   OpenViBE::CIdentifier(0x4eb92f81, 0x6ecda6b9)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_BufferDuration                                              OpenViBE::CIdentifier(0x7527d6e5, 0xb7a70339)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputParameterId_ExperimentInformationStream                                 OpenViBE::CIdentifier(0xa7f1d539, 0xec708539)
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedHeader                                                OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedBuffer                                                OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_AcquisitionStreamDecoder_OutputTriggerId_ReceivedEnd                                                   OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

#define OVP_GD_ClassId_Algorithm_MasterAcquisitionStreamEncoder                                                                 OpenViBE::CIdentifier(0x2d15e00b, 0x51414eb6)
#define OVP_GD_ClassId_Algorithm_MasterAcquisitionStreamEncoderDesc                                                             OpenViBE::CIdentifier(0xe6ec841d, 0x9e75a8fb)

#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelUnitData                                  OpenViBE::CIdentifier(0x19dc533c, 0x56301d0b)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_ChannelLocalisation                                    OpenViBE::CIdentifier(0x227e13f0, 0x206b44f9)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_EncodeChannelLocalisationData                          OpenViBE::CIdentifier(0x26ee1f81, 0x3db00d5d)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_StimulationSet                                         OpenViBE::CIdentifier(0x5b728d37, 0xfd088887)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_ChannelUnits                                           OpenViBE::CIdentifier(0x740060c2, 0x7d2b4f57)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectAge                                             OpenViBE::CIdentifier(0x9ef355e4, 0xc8531112)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectGender                                          OpenViBE::CIdentifier(0xa9056ae3, 0x57fe6af0)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalSamplingRate                                     OpenViBE::CIdentifier(0xb84ad0ca, 0x4f316dd3)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SubjectIdentifier                                      OpenViBE::CIdentifier(0xd5bb5231, 0x59389b72)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_BufferDuration                                         OpenViBE::CIdentifier(0xe1fc7385, 0x586a4f3f)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputParameterId_SignalMatrix                                           OpenViBE::CIdentifier(0xe9ac8077, 0xe369a51d)
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                   OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorit hm_AcquisitionStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputTriggerId_EncodeBuffer                                             OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorit hm_AcquisitionStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputTriggerId_EncodeEnd                                                OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorit hm_AcquisitionStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_MasterAcquisitionStreamEncoder_InputTriggerId_EncodeHeader                                             OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorit hm_AcquisitionStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Spectrum stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_SpectrumStreamDecoder                                                                          OpenViBE::CIdentifier(0x128202db, 0x449fc7a6)
#define OVP_GD_Algorithm_SpectrumStreamDecoder_InputParameterId_MemoryBufferToDecode                                            OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_FrequencyAbscissa                                              OpenViBE::CIdentifier(0x14A572E4, 0x5C405C8E)
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_SamplingRate                                                   OpenViBE::CIdentifier(0x68442C12, 0x0D9A46DE)
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputParameterId_Matrix                                                         OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d) // Duplicate of OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputTriggerId_ReceivedHeader                                                   OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputTriggerId_ReceivedBuffer                                                   OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_SpectrumStreamDecoder_OutputTriggerId_ReceivedEnd                                                      OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Stimulation stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_StimulationStreamDecoder                                                                       OpenViBE::CIdentifier(0xc8807f2b, 0x0813c5b1)
#define OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode                                         OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet                                              OpenViBE::CIdentifier(0xf46d0c19, 0x47306bea)
#define OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader                                                OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer                                                OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd                                                   OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Streamed matrix stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_StreamedMatrixStreamDecoder                                                                    OpenViBE::CIdentifier(0x7359d0db, 0x91784b21)
#define OVP_GD_Algorithm_StreamedMatrixStreamDecoder_InputParameterId_MemoryBufferToDecode                                      OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputParameterId_Matrix                                                   OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d) // Duplicate of OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix
#define OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedHeader                                             OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedBuffer                                             OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_StreamedMatrixStreamDecoder_OutputTriggerId_ReceivedEnd                                                OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Feature vector stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_FeatureVectorStreamDecoder                                                                     OpenViBE::CIdentifier(0xc2689ecc, 0x43b335c1)
#define OVP_GD_Algorithm_FeatureVectorStreamDecoder_InputParameterId_MemoryBufferToDecode                                       OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputParameterId_Matrix                                                    OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d) // Duplicate of OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix
#define OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedHeader                                              OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedBuffer                                              OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_FeatureVectorStreamDecoder_OutputTriggerId_ReceivedEnd                                                 OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Channel localisation stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ChannelLocalisationStreamDecoder                                                               OpenViBE::CIdentifier(0x8222f065, 0xb05d35cf)
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_InputParameterId_MemoryBufferToDecode                                 OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputParameterId_Matrix                                              OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d) // Duplicate of OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputParameterId_Dynamic                                             OpenViBE::CIdentifier(0xd20991fd, 0xa3153651)
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputTriggerId_ReceivedHeader                                        OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputTriggerId_ReceivedBuffer                                        OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_ChannelLocalisationStreamDecoder_OutputTriggerId_ReceivedEnd                                           OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Channel units stream decoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ChannelUnitsStreamDecoder                                                                      OpenViBE::CIdentifier(0x5f973ddf, 0x4a582daf)
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_InputParameterId_MemoryBufferToDecode                                        OpenViBE::CIdentifier(0x2f98ea3c, 0xfb0be096) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_InputParameterId_MemoryBufferToDecode
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_OutputParameterId_Dynamic                                                    OpenViBE::CIdentifier(0x31cf1c7a, 0x17475323)
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_OutputParameterId_Matrix                                                     OpenViBE::CIdentifier(0x79ef3123, 0x35e3ea4d) // Duplicate of OVP_GD_Algorithm_SignalStreamDecoder_OutputParameterId_Matrix
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_OutputTriggerId_ReceivedHeader                                               OpenViBE::CIdentifier(0x815234bf, 0xaabae5f2) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedHeader
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_OutputTriggerId_ReceivedBuffer                                               OpenViBE::CIdentifier(0xaa2738bf, 0xf7fe9fc3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedBuffer
#define OVP_GD_Algorithm_ChannelUnitsStreamDecoder_OutputTriggerId_ReceivedEnd                                                  OpenViBE::CIdentifier(0xc4aa114c, 0x628c2d77) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputTriggerId_ReceivedEnd

// -----------------------------------------------------
// Experiment information stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ExperimentInformationStreamEncoder                                                             OpenViBE::CIdentifier(0x56b354fe, 0xbf175468)
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectName                                        OpenViBE::CIdentifier(0x3d3826ea, 0xe8883815) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectName
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_ExperimentIdentifier                               OpenViBE::CIdentifier(0x40259641, 0x478c73de) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_ExperimentIdentifier
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_LaboratoryName                                     OpenViBE::CIdentifier(0x5ca80fa5, 0x774f01cb) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_LaboratoryName
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectGender                                      OpenViBE::CIdentifier(0x7d5059e8, 0xe4d8b38d) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectGender
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectIdentifier                                  OpenViBE::CIdentifier(0x97c5d20d, 0x203e65b3) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectIdentifier
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_TechnicianName                                     OpenViBE::CIdentifier(0xb8a94b68, 0x389393d9) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_TechnicianName
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_ExperimentDate                                     OpenViBE::CIdentifier(0xbc0266a2, 0x9c2935f1) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_ExperimentDate
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_SubjectAge                                         OpenViBE::CIdentifier(0xc36c6b08, 0x5227380a) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_SubjectAge
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_TechnicianIdentifier                               OpenViBE::CIdentifier(0xc8ecfbbc, 0x0dcda310) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_TechnicianIdentifier
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputParameterId_LaboratoryIdentifier                               OpenViBE::CIdentifier(0xe761d3d4, 0x44ba1ebf) // Duplicate of OVP_GD_Algorithm_ExperimentInformationStreamDecoder_OutputParameterId_LaboratoryIdentifier
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_OutputParameterId_EncodedMemoryBuffer                               OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputTriggerId_EncodeBuffer                                         OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputTriggerId_EncodeEnd                                            OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_ExperimentInformationStreamEncoder_InputTriggerId_EncodeHeader                                         OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Feature vector stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_FeatureVectorStreamEncoder                                                                     OpenViBE::CIdentifier(0x7ebe049d, 0xf777a602)
#define OVP_GD_Algorithm_FeatureVectorStreamEncoder_InputParameterId_Matrix                                                     OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix
#define OVP_GD_Algorithm_FeatureVectorStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                       OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_FeatureVectorStreamEncoder_InputTriggerId_EncodeBuffer                                                 OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_FeatureVectorStreamEncoder_InputTriggerId_EncodeEnd                                                    OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_FeatureVectorStreamEncoder_InputTriggerId_EncodeHeader                                                 OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Stimulation stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_StimulationStreamEncoder                                                                       OpenViBE::CIdentifier(0x6e86f7d5, 0xa4668108)
#define OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet                                               OpenViBE::CIdentifier(0x8565254c, 0x3a49268e)
#define OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                         OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer                                                   OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd                                                      OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader                                                   OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Channel units stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_ChannelUnitsStreamEncoder                                                                      OpenViBE::CIdentifier(0x2ca034fd, 0x5c051e86)
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_InputParameterId_Dynamic                                                     OpenViBE::CIdentifier(0x615f03b9, 0x4f6a320a)
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_InputParameterId_Matrix                                                      OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                        OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_InputTriggerId_EncodeBuffer                                                  OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_InputTriggerId_EncodeEnd                                                     OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_ChannelUnitsStreamEncoder_InputTriggerId_EncodeHeader                                                  OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

// -----------------------------------------------------
// Signal stream encoder
// -----------------------------------------------------

#define OVP_GD_ClassId_Algorithm_SignalStreamEncoder                                                                            OpenViBE::CIdentifier(0xc488ad3c, 0xeb2e36bf)
#define OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_SamplingRate                                                      OpenViBE::CIdentifier(0x998710ff, 0x2c5cca82)
#define OVP_GD_Algorithm_SignalStreamEncoder_InputParameterId_Matrix                                                            OpenViBE::CIdentifier(0xa3e9e5b0, 0xae756303) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputParameterId_Matrix
#define OVP_GD_Algorithm_SignalStreamEncoder_OutputParameterId_EncodedMemoryBuffer                                              OpenViBE::CIdentifier(0xa3d8b171, 0xf8734734) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_OutputParameterId_EncodedMemoryBuffer
#define OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeBuffer                                                        OpenViBE::CIdentifier(0x1b7076fd, 0x449bc70a) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeBuffer
#define OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeEnd                                                           OpenViBE::CIdentifier(0x3fc23508, 0x806753d8) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeEnd
#define OVP_GD_Algorithm_SignalStreamEncoder_InputTriggerId_EncodeHeader                                                        OpenViBE::CIdentifier(0x878eaf60, 0xf9d5303f) // Duplicate of OVP_GD_Algorithm_StreamedMatrixStreamEncoder_InputTriggerId_EncodeHeader

#endif // __OpenViBEPlugins_Global_Defines_H__
