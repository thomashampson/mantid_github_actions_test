// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidGeometry/MDGeometry/MDTypes.h"
// clang-format makes the reference data unreadable, therefore
// clang-format off
const std::vector<Mantid::coord_t> test_DataWS_ref = {
    4366, 4366, 0, 0, -0.09776273f, -0.09776273f, 0.10005156f, 0.0f,
    31461, 31461, 0, 1, -0.15959044f, -0.15959044f, 0.14884006f, 0.0f,
    33314, 33314, 0, 2, -0.224231616093f, -0.224231616093f, 0.189927174618f, 0.0f,
    32369, 32369, 0, 3, -0.291194311172f, -0.291194311172f, 0.223000198347f, 0.0f,
    31851, 31851, 0, 4, -0.359968893923f, -0.359968893923f, 0.247807429194f, 0.0f,
    30221, 30221, 0, 5, -0.430031948245f, -0.430031948245f, 0.264160069153f, 0.0f,
    26267, 26267, 0, 6, -0.500850251989f, -0.500850251989f, 0.271933664761f, 0.0f,
    26788, 26788, 0, 7, -0.571884835101f, -0.571884835101f, 0.27106905426f, 0.0f,
    29729, 29729, 0, 8, -0.642595081514f, -0.642595081514f, 0.26157281786f, 0.0f,
    30188, 30188, 0, 9, -0.712442843555f, -0.712442843555f, 0.243517227652f, 0.0f,
    28116, 28116, 0, 10, -0.78089653758f, -0.78089653758f, 0.217039697581f, 0.0f,
    30277, 30277, 0, 11, -0.847435189645f, -0.847435189645f, 0.182341737639f, 0.0f,
    20231, 20231, 0, 12, -0.911552400429f, -0.911552400429f, 0.13968742025f, 0.0f,
    24538, 24538, 0, 13, -0.972760199244f, -0.972760199244f, 0.089401370527f, 0.0f,
    16416, 16416, 0, 14, -1.03059275778f, -1.03059275778f, 0.0318662956709f, 0.0f,
    20225, 20225, 0, 15, -1.08460993535f, -1.08460993535f, -0.0324799276578f, 0.0f,
    19957, 19957, 0, 16, -1.13440062862f, -1.13440062862f, -0.103147585846f, 0.0f,
    19570, 19570, 0, 17, -1.17958590034f, -1.17958590034f, -0.179598855345f, 0.0f,
    20743, 20743, 0, 18, -1.21982186332f, -1.21982186332f, -0.261251895832f, 0.0f,
    22758, 22758, 0, 19, -1.25480229757f, -1.25480229757f, -0.347485278364f, 0.0f,
    23001, 23001, 0, 20, -1.28426098088f, -1.28426098088f, -0.437642714831f, 0.0f,
    21836, 21836, 0, 21, -1.30797371487f, -1.30797371487f, -0.531038052704f, 0.0f,
    23877, 23877, 0, 22, -1.32576003133f, -1.32576003133f, -0.626960497068f, 0.0f,
    13340, 13340, 0, 23, -1.33748456564f, -1.33748456564f, -0.724680020201f, 0.0f};


const std::vector<Mantid::coord_t> test_RawWS_ref = {
    4366, 4366, 0, 0, 4.27f, 87.54f, 1327.09f,
    31461, 31461, 0, 1, 6.77f, 87.54f, 1327.09f,
    33314, 33314, 0, 2, 9.27f, 87.54f, 1327.09f,
    32369, 32369, 0, 3, 11.77f, 87.54f, 1327.09f,
    31851, 31851, 0, 4, 14.27f, 87.54f, 1327.09f,
    30221, 30221, 0, 5, 16.77f, 87.54f, 1327.09f,
    26267, 26267, 0, 6, 19.27f, 87.54f, 1327.09f,
    26788, 26788, 0, 7, 21.77f, 87.54f, 1327.09f,
    29729, 29729, 0, 8, 24.27f, 87.54f, 1327.09f,
    30188, 30188, 0, 9, 26.77f, 87.54f, 1327.09f,
    28116, 28116, 0, 10, 29.27f, 87.54f, 1327.09f,
    30277, 30277, 0, 11, 31.77f, 87.54f, 1327.09f,
    20231, 20231, 0, 12, 34.27f, 87.54f, 1327.09f,
    24538, 24538, 0, 13, 36.77f, 87.54f, 1327.09f,
    16416, 16416, 0, 14, 39.27f, 87.54f, 1327.09f,
    20225, 20225, 0, 15, 41.77f, 87.54f, 1327.09f,
    19957, 19957, 0, 16, 44.27f, 87.54f, 1327.09f,
    19570, 19570, 0, 17, 46.77f, 87.54f, 1327.09f,
    20743, 20743, 0, 18, 49.27f, 87.54f, 1327.09f,
    22758, 22758, 0, 19, 51.77f, 87.54f, 1327.09f,
    23001, 23001, 0, 20, 54.27f, 87.54f, 1327.09f,
    21836, 21836, 0, 21, 56.77f, 87.54f, 1327.09f,
    23877, 23877, 0, 22, 59.27f, 87.54f, 1327.09f,
    13340, 13340, 0, 23, 61.77f, 87.54f, 1327.09f};

const std::vector<Mantid::coord_t> test_NormMonitor_ref = {
    8332872, 8332872, 0, 0, -0.09776273f, -0.09776273f, 0.10005156f, 0.0f,
    8332872, 8332872, 0, 1, -0.15959044f, -0.15959044f, 0.14884006f, 0.0f,
    8332872, 8332872, 0, 2, -0.224231616093f, -0.224231616093f, 0.189927174618f, 0.0f,
    8332872, 8332872, 0, 3, -0.291194311172f, -0.291194311172f, 0.223000198347f, 0.0f,
    8332872, 8332872, 0, 4, -0.359968893923f, -0.359968893923f, 0.247807429194f, 0.0f,
    8332872, 8332872, 0, 5, -0.430031948245f, -0.430031948245f, 0.264160069153f, 0.0f,
    8332872, 8332872, 0, 6, -0.500850251989f, -0.500850251989f, 0.271933664761f, 0.0f,
    8332872, 8332872, 0, 7, -0.571884835101f, -0.571884835101f, 0.27106905426f, 0.0f,
    8332872, 8332872, 0, 8, -0.642595081514f, -0.642595081514f, 0.26157281786f, 0.0f,
    8332872, 8332872, 0, 9, -0.712442843555f, -0.712442843555f, 0.243517227652f, 0.0f,
    8332872, 8332872, 0, 10, -0.78089653758f, -0.78089653758f, 0.217039697581f, 0.0f,
    8332872, 8332872, 0, 11, -0.847435189645f, -0.847435189645f, 0.182341737639f, 0.0f,
    8332872, 8332872, 0, 12, -0.911552400429f, -0.911552400429f, 0.13968742025f, 0.0f,
    8332872, 8332872, 0, 13, -0.972760199244f, -0.972760199244f, 0.089401370527f, 0.0f,
    8332872, 8332872, 0, 14, -1.03059275778f, -1.03059275778f, 0.0318662956709f, 0.0f,
    8332872, 8332872, 0, 15, -1.08460993535f, -1.08460993535f, -0.0324799276578f, 0.0f,
    8332872, 8332872, 0, 16, -1.13440062862f, -1.13440062862f, -0.103147585846f, 0.0f,
    8332872, 8332872, 0, 17, -1.17958590034f, -1.17958590034f, -0.179598855345f, 0.0f,
    8332872, 8332872, 0, 18, -1.21982186332f, -1.21982186332f, -0.261251895832f, 0.0f,
    8332872, 8332872, 0, 19, -1.25480229757f, -1.25480229757f, -0.347485278364f, 0.0f,
    8332872, 8332872, 0, 20, -1.28426098088f, -1.28426098088f, -0.437642714831f, 0.0f,
    8332872, 8332872, 0, 21, -1.30797371487f, -1.30797371487f, -0.531038052704f, 0.0f,
    8332872, 8332872, 0, 22, -1.32576003133f, -1.32576003133f, -0.626960497068f, 0.0f,
    8332872, 8332872, 0, 23, -1.33748456564f, -1.33748456564f, -0.724680020201f, 0.0f};


const std::vector<Mantid::coord_t> test_NormTime_ref = {
    600, 0, 0, 0, -0.09776273f, -0.09776273f, 0.10005156f, 0.0f,
    600, 0, 0, 1, -0.15959044f, -0.15959044f, 0.14884006f, 0.0f,
    600, 0, 0, 2, -0.224231616093f, -0.224231616093f, 0.189927174618f, 0.0f,
    600, 0, 0, 3, -0.291194311172f, -0.291194311172f, 0.223000198347f, 0.0f,
    600, 0, 0, 4, -0.359968893923f, -0.359968893923f, 0.247807429194f, 0.0f,
    600, 0, 0, 5, -0.430031948245f, -0.430031948245f, 0.264160069153f, 0.0f,
    600, 0, 0, 6, -0.500850251989f, -0.500850251989f, 0.271933664761f, 0.0f,
    600, 0, 0, 7, -0.571884835101f, -0.571884835101f, 0.27106905426f, 0.0f,
    600, 0, 0, 8, -0.642595081514f, -0.642595081514f, 0.26157281786f, 0.0f,
    600, 0, 0, 9, -0.712442843555f, -0.712442843555f, 0.243517227652f, 0.0f,
    600, 0, 0, 10, -0.78089653758f, -0.78089653758f, 0.217039697581f, 0.0f,
    600, 0, 0, 11, -0.847435189645f, -0.847435189645f, 0.182341737639f, 0.0f,
    600, 0, 0, 12, -0.911552400429f, -0.911552400429f, 0.13968742025f, 0.0f,
    600, 0, 0, 13, -0.972760199244f, -0.972760199244f, 0.089401370527f, 0.0f,
    600, 0, 0, 14, -1.03059275778f, -1.03059275778f, 0.0318662956709f, 0.0f,
    600, 0, 0, 15, -1.08460993535f, -1.08460993535f, -0.0324799276578f, 0.0f,
    600, 0, 0, 16, -1.13440062862f, -1.13440062862f, -0.103147585846f, 0.0f,
    600, 0, 0, 17, -1.17958590034f, -1.17958590034f, -0.179598855345f, 0.0f,
    600, 0, 0, 18, -1.21982186332f, -1.21982186332f, -0.261251895832f, 0.0f,
    600, 0, 0, 19, -1.25480229757f, -1.25480229757f, -0.347485278364f, 0.0f,
    600, 0, 0, 20, -1.28426098088f, -1.28426098088f, -0.437642714831f, 0.0f,
    600, 0, 0, 21, -1.30797371487f, -1.30797371487f, -0.531038052704f, 0.0f,
    600, 0, 0, 22, -1.32576003133f, -1.32576003133f, -0.626960497068f, 0.0f,
    600, 0, 0, 23, -1.33748456564f, -1.33748456564f, -0.724680020201f, 0.0f};


const std::vector<Mantid::coord_t> test_2ThetaLimits_ref = {
    32369, 32369, 0, 3, -0.291194311172f, -0.291194311172f, 0.223000198347f, 0.0f,
    31851, 31851, 0, 4, -0.359968893923f, -0.359968893923f, 0.247807429194f, 0.0f,
    30221, 30221, 0, 5, -0.430031948245f, -0.430031948245f, 0.264160069153f, 0.0f,
    26267, 26267, 0, 6, -0.500850251989f, -0.500850251989f, 0.271933664761f, 0.0f,
    26788, 26788, 0, 7, -0.571884835101f, -0.571884835101f, 0.27106905426f, 0.0f,
    29729, 29729, 0, 8, -0.642595081514f, -0.642595081514f, 0.26157281786f, 0.0f,
    30188, 30188, 0, 9, -0.712442843555f, -0.712442843555f, 0.243517227652f, 0.0f};

// reference vector, EPP is wrong, since no elastic channel is given
const std::vector<Mantid::coord_t> test_TOFWSData_ref = {
    0, 0, 0, 3, -0.0943541043211f, -0.0943541043211f, 2.51817307323f, -2.22473993186f,  //0
    0, 0, 0, 3, -0.105491719946f, -0.105491719946f, 2.18460421817f, -1.53897441518f,    //1
    0, 0, 0, 3, -0.115542738924f, -0.115542738924f, 1.88357866605f, -0.951095651067f,   //2
    0, 0, 0, 3, -0.124658779393f, -0.124658779393f, 1.61055549086f, -0.443322919741f,   //3
    0, 0, 0, 3, -0.132964505153f, -0.132964505153f, 1.36180104236f, -0.00173676689289f, //4
    0, 0, 0, 3, -0.140563360636f, -0.140563360636f, 1.13421718522f, 0.384685191257f,    //5
    0, 0, 0, 3, -0.147541901385f, -0.147541901385f, 0.925211602133f, 0.724762824962f,   //6
    0, 0, 0, 3, -0.153973105606f, -0.153973105606f, 0.732598613797f, 1.02562126597f,    //7
    0, 0, 0, 3, -0.159918935922f, -0.159918935922f, 0.55452245477f, 1.29306717609f,     //8
    0, 0, 0, 3, -0.165432342216f, -0.165432342216f, 0.389397289126f, 1.53187104043f,    //9
    0, 0, 0, 3, -0.170558842805f, -0.170558842805f, 0.235859854405f, 1.7459813825f,     //10
    0, 0, 0, 3, -0.175337784032f, -0.175337784032f, 0.0927317372921f, 1.93868903792f,   //11
    0, 0, 0, 3, -0.179803352064f, -0.179803352064f, -0.0410109295185f, 2.11275436309f,  //12
    0, 0, 0, 3, -0.183985391967f, -0.183985391967f, -0.166261998436f, 2.27050663742f,   //13
    0, 0, 0, 3, -0.187910075568f, -0.187910075568f, -0.283805309267f, 2.41392239469f,   //14
    0, 0, 0, 3, -0.1916004497f, -0.1916004497f, -0.394331109003f, 2.54468763779f,       //15
    0, 0, 0, 3, -0.1950768891f, -0.1950768891f, -0.498449616001f, 2.664247618f,         //16
    0, 0, 0, 3, -0.198357472759f, -0.198357472759f, -0.596702291619f, 2.77384694056f,   //17
    0, 0, 0, 3, -0.20145829841f, -0.20145829841f, -0.689571258983f, 2.87456208674f,     //18
    0, 0, 0, 3, -0.204393746692f, -0.204393746692f, -0.777487214755f, 2.96732794808f,   //19
    0, 0, 0, 3, -0.207176704155f, -0.207176704155f, -0.86083610789f, 3.05295960034f,    //20
    0, 0, 0, 3, -0.209818752378f, -0.209818752378f, -0.939964803903f, 3.13217026882f,   //21
    0, 0, 0, 3, -0.212330329085f, -0.212330329085f, -1.01518590999f, 3.20558622767f,    //22
    0, 0, 0, 3, -0.214720865951f, -0.214720865951f, -1.08678190253f, 3.27375921711f,    //23
    0, 0, 0, 3, -0.216998906964f, -0.216998906964f, -1.15500867189f, 3.33717683991f,    //24
    0, 0, 0, 3, -0.219172210459f, -0.219172210459f, -1.2200985783f, 3.39627130464f,     //25
    0, 0, 0, 3, -0.221247837392f, -0.221247837392f, -1.28226309565f, 3.45142680925f,    //26
    0, 0, 0, 3, -0.223232227977f, -0.223232227977f, -1.34169510674f, 3.5029858015f,     //27
    0, 0, 0, 3, -0.225131268429f, -0.225131268429f, -1.39857090231f, 3.55125430721f,    //28
    0, 0, 0, 3, -0.226950349283f, -0.226950349283f, -1.45305192753f, 3.59650648186f,    //29
    0, 0, 0, 3, -0.228694416494f, -0.228694416494f, -1.50528631254f, 3.63898851198f,    //30
    0, 0, 0, 3, -0.230368016343f, -0.230368016343f, -1.55541021734f, 3.67892197067f,    //31
    0, 0, 0, 3, -0.231975335009f, -0.231975335009f, -1.60354901701f, 3.71650671254f,    //32
    0, 0, 0, 3, -0.233520233533f, -0.233520233533f, -1.64981834872f, 3.75192337916f,    //33
    0, 0, 0, 3, -0.23500627878f, -0.23500627878f, -1.69432503923f, 3.78533557363f,      //34
    0, 0, 0, 3, -0.236436770934f, -0.236436770934f, -1.73716792823f, 3.81689175332f,    //35
    0, 0, 0, 3, -0.237814767963f, -0.237814767963f, -1.77843860111f, 3.84672688183f,    //36
    0, 0, 0, 3, -0.239143107441f, -0.239143107441f, -1.81822204254f, 3.87496387446f,    //37
    0, 0, 0, 3, -0.240424426052f, -0.240424426052f, -1.85659722056f, 3.90171486617f,    //38
    0, 0, 0, 3, -0.24166117706f, -0.24166117706f, -1.89363760977f, 3.92708232664f,      //39
    0, 0, 0, 3, -0.242855645982f, -0.242855645982f, -1.92941166089f, 3.95116004298f,    //40
    1, 1, 0, 3, -0.244009964688f, -0.244009964688f, -1.9639832229f, 3.97403398783f,     //41
    1, 1, 0, 3, -0.245126124099f, -0.245126124099f, -1.99741192336f, 3.99578308789f,    //42
    0, 0, 0, 3, -0.246205985642f, -0.246205985642f, -2.0297535116f, 4.01647990566f,
    2, 2, 0, 3, -0.247251291615f, -0.247251291615f, -2.06106016902f, 4.03619124547f,
    0, 0, 0, 3, -0.248263674566f, -0.248263674566f, -2.09138078999f, 4.05497869322f,
    2, 2, 0, 3, -0.249244665798f, -0.249244665798f, -2.12076123667f, 4.07289909803f,
    6, 6, 0, 3, -0.250195703099f, -0.250195703099f, -2.14924457046f, 4.09000500276f,	//EPP
    3, 3, 0, 3, -0.251118137774f, -0.251118137774f, -2.17687126264f, 4.10634502964f,
    0, 0, 0, 3, -0.252013241051f, -0.252013241051f, -2.20367938617f, 4.12196422612f,
    0, 0, 0, 3, -0.252882209926f, -0.252882209926f, -2.22970479075f, 4.1369043757f,
    0, 0, 0, 3, -0.253726172503f, -0.253726172503f, -2.25498126284f, 4.15120427767f,
    0, 0, 0, 3, -0.254546192879f, -0.254546192879f, -2.27954067188f, 4.16489999925f,
    0, 0, 0, 3, -0.255343275622f, -0.255343275622f, -2.30341310445f, 4.17802510323f,
    0, 0, 0, 3, -0.256118369875f, -0.256118369875f, -2.32662698715f, 4.19061085383f,
    0, 0, 0, 3, -0.256872373129f, -0.256872373129f, -2.34920919958f, 4.20268640299f,
    0, 0, 0, 3, -0.257606134684f, -0.257606134684f, -2.37118517811f, 4.2142789594f,
    0, 0, 0, 3, -0.258320458847f, -0.258320458847f, -2.39257901152f, 4.22541394183f,
    0, 0, 0, 3, -0.259016107869f, -0.259016107869f, -2.41341352902f, 4.23611511866f,
    0, 0, 0, 3, -0.259693804658f, -0.259693804658f, -2.43371038155f, 4.24640473475f,
    0, 0, 0, 3, -0.260354235286f, -0.260354235286f, -2.45349011683f, 4.25630362721f,
    0, 0, 0, 3, -0.260998051308f, -0.260998051308f, -2.4727722487f, 4.2658313309f,
    0, 0, 0, 3, -0.2616258719f, -0.2616258719f, -2.49157532139f, 4.27500617494f,
    0, 0, 0, 3, -0.262238285852f, -0.262238285852f, -2.50991696898f, 4.2838453709f,
    0, 0, 0, 3, -0.262835853406f, -0.262835853406f, -2.52781397058f, 4.2923650936f,
    0, 0, 0, 3, -0.263419107964f, -0.263419107964f, -2.54528230147f, 4.30058055512f,
    0, 0, 0, 3, -0.26398855768f, -0.26398855768f, -2.56233718075f, 4.3085060728f,
    0, 0, 0, 3, -0.264544686935f, -0.264544686935f, -2.57899311548f, 4.31615513161f,
    0, 0, 0, 3, -0.265087957711f, -0.265087957711f, -2.5952639419f, 4.32354044159f,
    0, 0, 0, 3, -0.265618810868f, -0.265618810868f, -2.61116286371f, 4.33067399066f,
    0, 0, 0, 3, -0.266137667344f, -0.266137667344f, -2.62670248786f, 4.33756709332f,
    0, 0, 0, 3, -0.266644929262f, -0.266644929262f, -2.64189485783f, 4.34423043551f,
    0, 0, 0, 3, -0.267140980972f, -0.267140980972f, -2.65675148482f, 4.35067411606f,
    0, 0, 0, 3, -0.267626190022f, -0.267626190022f, -2.67128337679f, 4.35690768501f,
    0, 0, 0, 3, -0.268100908065f, -0.268100908065f, -2.6855010657f, 4.36294017896f,
    0, 0, 0, 3, -0.268565471711f, -0.268565471711f, -2.69941463291f, 4.3687801539f,
    0, 0, 0, 3, -0.269020203322f, -0.269020203322f, -2.7130337331f, 4.37443571549f,
    0, 0, 0, 3, -0.269465411759f, -0.269465411759f, -2.72636761653f, 4.37991454727f,
    0, 0, 0, 3, -0.269901393077f, -0.269901393077f, -2.73942515004f, 4.38522393675f,
    0, 0, 0, 3, -0.27032843119f, -0.27032843119f, -2.75221483671f, 4.39037079963f,
    0, 0, 0, 3, -0.270746798477f, -0.270746798477f, -2.76474483431f, 4.39536170235f,
    0, 0, 0, 3, -0.271156756372f, -0.271156756372f, -2.77702297266f, 4.40020288306f};

// reference vector, EPP should be near dE=0
const std::vector<Mantid::coord_t> test_TOFWSDataRotateEPP_ref = {
    0, 0, 0, 4, 0.0354153287653f, 0.0354153287653f, 3.30960431031f, -2.22473993186f,
    0, 0, 0, 4, 0.0119271789404f, 0.0119271789404f, 3.14246833446f, -1.53897441518f,
    1, 1, 0, 4, -0.00926944407241f, -0.00926944407241f, 2.99163830747f, -0.951095651067f,
    1, 1, 0, 4, -0.0284942882003f, -0.0284942882003f, 2.85483898067f, -0.443322919741f,
    1, 1, 0, 4, -0.0460102572946f, -0.0460102572946f, 2.73019959403f, -0.00173676689289f,
    3, 3, 0, 4, -0.0620355056149f, -0.0620355056149f, 2.61616781476f, 0.384685191257f,
    1, 1, 0, 4, -0.0767525703988f, -0.0767525703988f, 2.51144475217f, 0.724762824962f,
    1, 1, 0, 4, -0.0903153555918f, -0.0903153555918f, 2.41493526311f, 1.02562126597f,
    0, 0, 0, 4, -0.102854534355f, -0.102854534355f, 2.32570950908f, 1.29306717609f,
    0, 0, 0, 4, -0.114481772845f, -0.114481772845f, 2.24297290079f, 1.53187104043f,
    0, 0, 0, 4, -0.125293064774f, -0.125293064774f, 2.16604237028f, 1.7459813825f,
    0, 0, 0, 4, -0.135371387759f, -0.135371387759f, 2.09432746896f, 1.93868903792f,
    0, 0, 0, 4, -0.144788837105f, -0.144788837105f, 2.02731518411f, 2.11275436309f,
    0, 0, 0, 4, -0.153608353159f, -0.153608353159f, 1.96455764752f, 2.27050663742f,
    0, 0, 0, 4, -0.161885129764f, -0.161885129764f, 1.90566211317f, 2.41392239469f,
    0, 0, 0, 4, -0.169667770453f, -0.169667770453f, 1.85028273012f, 2.54468763779f,
    0, 0, 0, 4, -0.176999243565f, -0.176999243565f, 1.7981137461f, 2.664247618f,
    0, 0, 0, 4, -0.183917675938f, -0.183917675938f, 1.74888385976f, 2.77384694056f,
    0, 0, 0, 4, -0.190457016127f, -0.190457016127f, 1.70235150145f, 2.87456208674f,
    0, 0, 0, 4, -0.196647591505f, -0.196647591505f, 1.65830086891f, 2.96732794808f,
    0, 0, 0, 4, -0.202516578553f, -0.202516578553f, 1.61653858092f, 3.05295960034f,
    0, 0, 0, 4, -0.208088401699f, -0.208088401699f, 1.57689083915f, 3.13217026882f,
    0, 0, 0, 4, -0.213385073084f, -0.213385073084f, 1.53920101056f, 3.20558622767f,
    0, 0, 0, 4, -0.218426483198f, -0.218426483198f, 1.50332755925f, 3.27375921711f,
    0, 0, 0, 4, -0.223230650484f, -0.223230650484f, 1.46914227036f, 3.33717683991f,
    0, 0, 0, 4, -0.227813936514f, -0.227813936514f, 1.43652871888f, 3.39627130464f,
    0, 0, 0, 4, -0.232191232162f, -0.232191232162f, 1.405380945f, 3.45142680925f,
    0, 0, 0, 4, -0.236376119209f, -0.236376119209f, 1.37560230404f, 3.5029858015f,
    0, 0, 0, 4, -0.240381011115f, -0.240381011115f, 1.34710446484f, 3.55125430721f,
    0, 0, 0, 4, -0.244217275993f, -0.244217275993f, 1.31980653466f, 3.59650648186f,
    0, 0, 0, 4, -0.247895344381f, -0.247895344381f, 1.29363429232f, 3.63898851198f,
    0, 0, 0, 4, -0.251424803946f, -0.251424803946f, 1.26851951432f, 3.67892197067f,
    0, 0, 0, 4, -0.254814482933f, -0.254814482933f, 1.24439938099f, 3.71650671254f,
    0, 0, 0, 4, -0.258072523902f, -0.258072523902f, 1.22121595187f, 3.75192337916f,
    0, 0, 0, 4, -0.261206449024f, -0.261206449024f, 1.198915701f, 3.78533557363f,
    0, 0, 0, 4, -0.264223218067f, -0.264223218067f, 1.17744910437f, 3.81689175332f,
    0, 0, 0, 4, -0.267129279989f, -0.267129279989f, 1.15677027275f, 3.84672688183f,
    0, 0, 0, 4, -0.269930618959f, -0.269930618959f, 1.13683662426f, 3.87496387446f,
    0, 0, 0, 4, -0.272632795488f, -0.272632795488f, 1.11760859164f, 3.90171486617f,
    0, 0, 0, 4, -0.275240983267f, -0.275240983267f, 1.09904936015f, 3.92708232664f,
    0, 0, 0, 4, -0.277760002234f, -0.277760002234f, 1.08112463231f, 3.95116004298f,
    0, 0, 0, 4, -0.280194348295f, -0.280194348295f, 1.06380241632f, 3.97403398783f,
    0, 0, 0, 4, -0.282548220105f, -0.282548220105f, 1.04705283558f, 3.99578308789f,
    0, 0, 0, 4, -0.284825543238f, -0.284825543238f, 1.03084795664f, 4.01647990566f,
    0, 0, 0, 4, -0.287029992032f, -0.287029992032f, 1.01516163384f, 4.03619124547f,
    0, 0, 0, 4, -0.289165009367f, -0.289165009367f, 0.999969368441f, 4.05497869322f,
    0, 0, 0, 4, -0.291233824614f, -0.291233824614f, 0.985248181042f, 4.07289909803f,
    0, 0, 0, 4, -0.293239469931f, -0.293239469931f, 0.970976495549f, 4.09000500276f,
    0, 0, 0, 4, -0.295184795087f, -0.295184795087f, 0.957134033679f, 4.10634502964f,
    0, 0, 0, 4, -0.297072480979f, -0.297072480979f, 0.943701718828f, 4.12196422612f,
    0, 0, 0, 4, -0.298905051955f, -0.298905051955f, 0.930661588352f, 4.1369043757f,
    0, 0, 0, 4, -0.300684887075f, -0.300684887075f, 0.91799671343f, 4.15120427767f,
    0, 0, 0, 4, -0.302414230419f, -0.302414230419f, 0.905691125739f, 4.16489999925f,
    0, 0, 0, 4, -0.304095200523f, -0.304095200523f, 0.893729750291f, 4.17802510323f,
    0, 0, 0, 4, -0.305729799037f, -0.305729799037f, 0.882098343821f, 4.19061085383f,
    0, 0, 0, 4, -0.307319918681f, -0.307319918681f, 0.870783438208f, 4.20268640299f,
    0, 0, 0, 4, -0.308867350548f, -0.308867350548f, 0.859772288449f, 4.2142789594f,
    0, 0, 0, 4, -0.310373790843f, -0.310373790843f, 0.849052824778f, 4.22541394183f,
    0, 0, 0, 4, -0.311840847078f, -0.311840847078f, 0.838613608522f, 4.23611511866f,
    0, 0, 0, 4, -0.313270043798f, -0.313270043798f, 0.828443791396f, 4.24640473475f,
    0, 0, 0, 4, -0.314662827862f, -0.314662827862f, 0.81853307789f, 4.25630362721f,
    0, 0, 0, 4, -0.316020573333f, -0.316020573333f, 0.808871690511f, 4.2658313309f,
    0, 0, 0, 4, -0.317344585997f, -0.317344585997f, 0.799450337601f, 4.27500617494f,
    0, 0, 0, 4, -0.318636107553f, -0.318636107553f, 0.790260183535f, 4.2838453709f,
    0, 0, 0, 4, -0.319896319496f, -0.319896319496f, 0.781292821082f, 4.2923650936f,
    0, 0, 0, 4, -0.321126346721f, -0.321126346721f, 0.772540245755f, 4.30058055512f,
    0, 0, 0, 4, -0.322327260876f, -0.322327260876f, 0.763994831973f, 4.3085060728f,
    0, 0, 0, 4, -0.323500083472f, -0.323500083472f, 0.755649310912f, 4.31615513161f,
    0, 0, 0, 4, -0.324645788783f, -0.324645788783f, 0.747496749875f, 4.32354044159f,
    0, 0, 0, 4, -0.325765306543f, -0.325765306543f, 0.739530533091f, 4.33067399066f,
    0, 0, 0, 4, -0.326859524467f, -0.326859524467f, 0.731744343804f, 4.33756709332f,
    0, 0, 0, 4, -0.327929290594f, -0.327929290594f, 0.724132147574f, 4.34423043551f,
    0, 0, 0, 4, -0.328975415481f, -0.328975415481f, 0.716688176675f, 4.35067411606f,
    0, 0, 0, 4, -0.329998674249f, -0.329998674249f, 0.709406915523f, 4.35690768501f,
    0, 0, 0, 4, -0.330999808504f, -0.330999808504f, 0.702283087044f, 4.36294017896f,
    0, 0, 0, 4, -0.331979528123f, -0.331979528123f, 0.695311639923f, 4.3687801539f,
    0, 0, 0, 4, -0.332938512934f, -0.332938512934f, 0.688487736656f, 4.37443571549f,
    0, 0, 0, 4, -0.333877414294f, -0.333877414294f, 0.681806742359f, 4.37991454727f,
    0, 0, 0, 4, -0.334796856559f, -0.334796856559f, 0.675264214264f, 4.38522393675f,
    0, 0, 0, 4, -0.335697438469f, -0.335697438469f, 0.668855891874f, 4.39037079963f,
    0, 0, 0, 4, -0.336579734452f, -0.336579734452f, 0.662577687705f, 4.39536170235f,
    0, 0, 0, 4, -0.337444295843f, -0.337444295843f, 0.656425678595f, 4.40020288306f};

// clang-format on