(window.webpackJsonp=window.webpackJsonp||[]).push([[23],{379:function(e,t,a){e.exports=a.p+"assets/img/OpenMQTTGateway_serial_2G_1.55e6a1b4.jpg"},380:function(e,t,a){e.exports=a.p+"assets/img/OpenMQTTGateway_serial_2G_2.262163e3.jpg"},428:function(e,t,a){"use strict";a.r(t);var s=a(14),n=Object(s.a)({},(function(){var e=this,t=e._self._c;return t("ContentSlotsDistributor",{attrs:{"slot-key":e.$parent.slotKey}},[t("h1",{attrs:{id:"gsm-2g-gateway"}},[t("a",{staticClass:"header-anchor",attrs:{href:"#gsm-2g-gateway"}},[e._v("#")]),e._v(" GSM 2G gateway")]),e._v(" "),t("h2",{attrs:{id:"receiving-sms-from-2g"}},[t("a",{staticClass:"header-anchor",attrs:{href:"#receiving-sms-from-2g"}},[e._v("#")]),e._v(" Receiving SMS from 2G")]),e._v(" "),t("p",[e._v("Subscribe to all the messages with mosquitto or open your MQTT client software:\n"),t("code",[e._v("mosquitto_sub -t +/# -v")])]),e._v(" "),t("p",[e._v("Generate your SMS by sending an SMS to the phone number of the A6/A7 SIM card you will see :")]),e._v(" "),t("div",{staticClass:"language- line-numbers-mode"},[t("pre",{pre:!0,attrs:{class:"language-text"}},[t("code",[e._v('home/OpenMQTTGateway/2GtoMQTT {"phone":"+33123456789","date":"2018/04/22,16:17:51+02","message":"Hello OpenMQTTGateway"}\n')])]),e._v(" "),t("div",{staticClass:"line-numbers-wrapper"},[t("span",{staticClass:"line-number"},[e._v("1")]),t("br")])]),t("h2",{attrs:{id:"send-data-by-mqtt-to-send-an-sms-message-with-a6-7"}},[t("a",{staticClass:"header-anchor",attrs:{href:"#send-data-by-mqtt-to-send-an-sms-message-with-a6-7"}},[e._v("#")]),e._v(" Send data by MQTT to send an SMS message with A6/7")]),e._v(" "),t("p",[t("code",[e._v('sudo mosquitto_pub -t home/OpenMQTTGateway/commands/MQTTto2G -m \'{"phone":"+33123456789","date":"2018/04/22,16:17:51+02","message":"hello"}\'')])]),e._v(" "),t("p",[e._v('This command will send by SMS the sentence "hello" and use the phone number 0123456789')]),e._v(" "),t("p",[e._v("Arduino IDE serial data received when publishing data by MQTT")]),e._v(" "),t("p",[t("img",{attrs:{src:a(379),alt:""}})]),e._v(" "),t("p",[e._v("Arduino IDE serial data received when receiving SMS")]),e._v(" "),t("p",[t("img",{attrs:{src:a(380),alt:""}})])])}),[],!1,null,null,null);t.default=n.exports}}]);