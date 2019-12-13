/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2019 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ags/server/security/ags_xml_certificate.h>

#include <ags/server/security/ags_certificate.h>

#include <libxml/parser.h>
#include <libxml/xlink.h>
#include <libxml/xpath.h>
#include <libxml/valid.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

void ags_xml_certificate_class_init(AgsXmlCertificateClass *xml_certificate);
void ags_xml_certificate_certificate_interface_init(AgsCertificateInterface *certificate);
void ags_xml_certificate_init(AgsXmlCertificate *xml_certificate);
void ags_xml_certificate_finalize(GObject *gobject);

gchar** ags_xml_certificate_get_cert_uuid(AgsCertificate *certificate,
					  GObject *security_context,
					  gchar *user_uuid,
					  gchar *security_token,
					  GError **error);
void ags_xml_certificate_set_domain(AgsCertificate *certificate,
				    GObject *security_context,
				    gchar *user_uuid,
				    gchar *security_token,
				    gchar *cert_uuid,
				    gchar *domain,
				    GError **error);
gchar* ags_xml_certificate_get_domain(AgsCertificate *certificate,
				      GObject *security_context,
				      gchar *user_uuid,
				      gchar *security_token,
				      gchar *cert_uuid,
				      GError **error);
void ags_xml_certificate_set_key_type(AgsCertificate *certificate,
				      GObject *security_context,
				      gchar *user_uuid,
				      gchar *security_token,
				      gchar *cert_uuid,
				      gchar *key_type,
				      GError **error);
gchar* ags_xml_certificate_get_key_type(AgsCertificate *certificate,
					GObject *security_context,
					gchar *user_uuid,
					gchar *security_token,
					gchar *cert_uuid,
					GError **error);
void ags_xml_certificate_set_public_key_file(AgsCertificate *certificate,
					     GObject *security_context,
					     gchar *user_uuid,
					     gchar *security_token,
					     gchar *cert_uuid,
					     gchar *public_key_file,
					     GError **error);
gchar* ags_xml_certificate_get_public_key_file(AgsCertificate *certificate,
					       GObject *security_context,
					       gchar *user_uuid,
					       gchar *security_token,
					       gchar *cert_uuid,
					       GError **error);
void ags_xml_certificate_set_private_key_file(AgsCertificate *certificate,
					      GObject *security_context,
					      gchar *user_uuid,
					      gchar *security_token,
					      gchar *cert_uuid,
					      gchar *private_key_file,
					      GError **error);
gchar* ags_xml_certificate_get_private_key_file(AgsCertificate *certificate,
						GObject *security_context,
						gchar *user_uuid,
						gchar *security_token,
						gchar *cert_uuid,
						GError **error);

/**
 * SECTION:ags_xml_certificate
 * @short_description: certificate by XML file
 * @title: AgsXmlCertificate
 * @section_id:
 * @include: ags/server/security/ags_xml_certificate.h
 *
 * The #AgsXmlCertificate is an object to verify certificates.
 */

static gpointer ags_xml_certificate_parent_class = NULL;

GType
ags_xml_certificate_get_type()
{
  static volatile gsize g_define_type_id__volatile = 0;

  if(g_once_init_enter (&g_define_type_id__volatile)){
    GType ags_type_xml_certificate = 0;

    static const GTypeInfo ags_xml_certificate_info = {
      sizeof (AgsXmlCertificateClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) ags_xml_certificate_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof (AgsXmlCertificate),
      0,    /* n_preallocs */
      (GInstanceInitFunc) ags_xml_certificate_init,
    };

    static const GInterfaceInfo ags_certificate_interface_info = {
      (GInterfaceInitFunc) ags_xml_certificate_certificate_interface_init,
      NULL, /* interface_finalize */
      NULL, /* interface_data */
    };
    
    ags_type_xml_certificate = g_type_register_static(G_TYPE_OBJECT,
						      "AgsXmlCertificate",
						      &ags_xml_certificate_info,
						      0);

    g_type_add_interface_static(ags_type_xml_certificate,
				AGS_TYPE_CERTIFICATE,
				&ags_certificate_interface_info);

    g_once_init_leave(&g_define_type_id__volatile, ags_type_xml_certificate);
  }

  return g_define_type_id__volatile;
}

void
ags_xml_certificate_class_init(AgsXmlCertificateClass *xml_certificate)
{
  GObjectClass *gobject;
  GParamSpec *param_spec;
  
  ags_xml_certificate_parent_class = g_type_class_peek_parent(xml_certificate);

  /* GObjectClass */
  gobject = (GObjectClass *) xml_certificate;

  gobject->finalize = ags_xml_certificate_finalize;
}

void
ags_xml_certificate_certificate_interface_init(AgsCertificateInterface *certificate)
{
  certificate->get_cert_uuid = ags_xml_certificate_get_cert_uuid;

  certificate->set_domain = ags_xml_certificate_set_domain;
  certificate->get_domain = ags_xml_certificate_get_domain;

  certificate->set_key_type = ags_xml_certificate_set_key_type;
  certificate->get_key_type = ags_xml_certificate_get_key_type;

  certificate->set_public_key_file = ags_xml_certificate_set_public_key_file;
  certificate->get_public_key_file = ags_xml_certificate_get_public_key_file;

  certificate->set_private_key_file = ags_xml_certificate_set_private_key_file;
  certificate->get_private_key_file = ags_xml_certificate_get_private_key_file;
}

void
ags_xml_certificate_init(AgsXmlCertificate *xml_certificate)
{
  g_rec_mutex_init(&(xml_certificate->obj_mutex));

  xml_certificate->filename = NULL;
  xml_certificate->encoding = NULL;
  xml_certificate->dtd = NULL;

  xml_certificate->doc = NULL;
  xml_certificate->root_node = NULL;
}

void
ags_xml_certificate_finalize(GObject *gobject)
{
  AgsXmlCertificate *xml_certificate;

  xml_certificate = AGS_XML_CERTIFICATE(gobject);

  G_OBJECT_CLASS(ags_xml_certificate_parent_class)->finalize(gobject);
}

gchar**
ags_xml_certificate_get_cert_uuid(AgsCertificate *certificate,
				  GObject *security_context,
				  gchar *user_uuid,
				  gchar *security_token,
				  GError **error)
{
  //TODO:JK: implement me
}

void
ags_xml_certificate_set_domain(AgsCertificate *certificate,
			       GObject *security_context,
			       gchar *user_uuid,
			       gchar *security_token,
			       gchar *cert_uuid,
			       gchar *domain,
			       GError **error)
{
  //TODO:JK: implement me
}

gchar*
ags_xml_certificate_get_domain(AgsCertificate *certificate,
			       GObject *security_context,
			       gchar *user_uuid,
			       gchar *security_token,
			       gchar *cert_uuid,
			       GError **error)
{
  //TODO:JK: implement me
}

void
ags_xml_certificate_set_key_type(AgsCertificate *certificate,
				 GObject *security_context,
				 gchar *user_uuid,
				 gchar *security_token,
				 gchar *cert_uuid,
				 gchar *key_type,
				 GError **error)
{
  //TODO:JK: implement me
}

gchar*
ags_xml_certificate_get_key_type(AgsCertificate *certificate,
				 GObject *security_context,
				 gchar *user_uuid,
				 gchar *security_token,
				 gchar *cert_uuid,
				 GError **error)
{
  //TODO:JK: implement me
}

void
ags_xml_certificate_set_public_key_file(AgsCertificate *certificate,
					GObject *security_context,
					gchar *user_uuid,
					gchar *security_token,
					gchar *cert_uuid,
					gchar *public_key_file,
					GError **error)
{
  //TODO:JK: implement me
}

gchar*
ags_xml_certificate_get_public_key_file(AgsCertificate *certificate,
					GObject *security_context,
					gchar *user_uuid,
					gchar *security_token,
					gchar *cert_uuid,
					GError **error)
{
  //TODO:JK: implement me
}

void
ags_xml_certificate_set_private_key_file(AgsCertificate *certificate,
					 GObject *security_context,
					 gchar *user_uuid,
					 gchar *security_token,
					 gchar *cert_uuid,
					 gchar *private_key_file,
					 GError **error)
{
  //TODO:JK: implement me
}

gchar*
ags_xml_certificate_get_private_key_file(AgsCertificate *certificate,
					 GObject *security_context,
					 gchar *user_uuid,
					 gchar *security_token,
					 gchar *cert_uuid,
					 GError **error)
{
  //TODO:JK: implement me
}

/**
 * ags_xml_certificate_new:
 *
 * Create #AgsXmlCertificate.
 *
 * Returns: the new #AgsXmlCertificate instance
 *
 * Since: 2.0.0
 */
AgsXmlCertificate*
ags_xml_certificate_new()
{
  AgsXmlCertificate *xml_certificate;

  xml_certificate = (AgsXmlCertificate *) g_object_new(AGS_TYPE_XML_CERTIFICATE,
						       NULL);

  return(xml_certificate);
}
