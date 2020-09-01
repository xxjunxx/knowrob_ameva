#include "knowrob_ameva.pb.h"
#include <SWI-cpp.h>
#include <iostream>
#include <libwebsockets.h>
#include <stdio.h>
#include <thread>
#include <queue>
#include <string>

class RequestTask 
{
public:
	int client_id;
	std::string msg;
	RequestTask(int i, std::string m) : client_id(i), msg(m) {}
};

const char* LOG_LABEL = "[AMEVA] ";      // log label
static const int CLIENT_NUM = 128;       // maximun connected client

static int interrupted;                  // stop server
static std::queue<RequestTask*> queue;   // messages to send
static struct lws *clients[CLIENT_NUM];  // unreal client 
static int client_id = 0;                // client id


// HTTP handler
static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	return 0;
}

// Websocket handler
static int callback_plwebsocket( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	int m;
	switch( reason )
	{
		case LWS_CALLBACK_ESTABLISHED:
		{
			// Save connected client
			std::cout << LOG_LABEL << "A new client connected. Client id : " << client_id << "\n";
			clients[client_id++] = wsi;
			break;
		}	
		case LWS_CALLBACK_RECEIVE:
		{
			// Print received from clients
			int id = -1;
			for (int i = 0; i < CLIENT_NUM; i++) 
			{
				if ( clients[i]== wsi) 
				{
					id = i;
					break;
				} 
			}
			std::cout << LOG_LABEL << "Message from client - " << id << " : " << (char*) in <<"\n";
			break;
		}
		case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			// Check messages queue and send message
			if (queue.size() > 0)
			{
				RequestTask* task = queue.front();
				// LWS_PRE bytes before buffer for adding protocal info
				std::string padding(LWS_PRE, ' ');
				padding += task->msg;
				m = lws_write(wsi, (unsigned char*)&padding[LWS_PRE], padding.size(), LWS_WRITE_TEXT);
				queue.pop();
				delete task;
				break;
			}
			break;
		}
		case LWS_CALLBACK_CLOSED:
		{
			// Close connection
			int id = -1;
			for (int i = 0; i < CLIENT_NUM; i++) 
			{
				if ( clients[i]== wsi) 
				{
					clients[i] = NULL;
					id = i;
					break;
				} 
			}
			std::cout << LOG_LABEL << "Client - " << id << " disconnected.\n";
			break;
		}
		default:
			break;
	}

	return 0;
}

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   	/* name */
		callback_http, 	/* callback */
		0,             	/* No per session data. */
		128,           	/* max frame size / rx buffer */
	},
	{
		"prolog_websocket",
		callback_plwebsocket,
		0,
		128,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};


void server_thread(int port) 
{
	
	struct lws_context_creation_info info;
	struct lws_context *context;
	int n = 0;
	
	// websocket handler parameter
	memset( &info, 0, sizeof(info) );
	info.port = port;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	// create websocket handler
	context = lws_create_context( &info );

	if (!context) 
	{
		return;
	}

	while( n >= 0 && !interrupted )
	{
		// service andy pending websocket activity, non-blocking
		n = lws_service( context, /* timeout_ms = */ 50 );	
		
		// request a callback to write message
		if (queue.size() > 0)
		{
			RequestTask* task = queue.front();
			if (clients[task->client_id] == NULL) 
			{
				std::cout << LOG_LABEL << "client " << task->client_id << " is not connected\n";
				queue.pop();
				delete task;
				continue;
			} 
			lws_callback_on_writable(clients[task->client_id]);
		}	
	}
	lws_context_destroy( context );
}

sl_pb::MarkerType get_mesh_type(char* type)
{

	if (strcmp(type, "sphere") == 0)
	{
		return sl_pb::Sphere;
	}
	else if (strcmp(type, "cyclinder") == 0)
	{
		return sl_pb::Cylinder;
	}
	else if (strcmp(type, "arrow") == 0)
	{
		return sl_pb::Arrow;
	}
	else if (strcmp(type, "axis") == 0)
	{
		return sl_pb::Axis;
	}
	return sl_pb::Box;
}

PREDICATE(ue_start_srv, 0)
{ 
	std::thread (server_thread, 8080).detach();
	return TRUE;
}

PREDICATE(ue_show_clients, 0)
{ 
	std::cout << LOG_LABEL << "Connected clients:\n";
	for (int i = 0; i < CLIENT_NUM; i++) 
	{
		if (clients[i] != NULL) 
		{
			std::cout << "client - " << i << "\n";
		}
	}
	return TRUE;
}


PREDICATE(ue_set_task, 2)
{
	sl_pb::KRAmevaEvent ameva_event;
	ameva_event.set_functocall(ameva_event.SetTask);
	sl_pb::SetTaskParams* set_task_params = ameva_event.mutable_settaskparam();
	set_task_params->set_task((char*)A2);
	std::string proto_str = ameva_event.SerializeAsString();
	RequestTask* task = new RequestTask((int) A1, proto_str);
	queue.push(task);
	return TRUE;
}

PREDICATE(ue_set_episode, 2)
{
	sl_pb::KRAmevaEvent ameva_event;
	ameva_event.set_functocall(ameva_event.SetEpisode);
	sl_pb::SetEpisodeParams* set_episode_params = ameva_event.mutable_setepisodeparams();
	set_episode_params->set_episode((char*)A2);
	std::string proto_str = ameva_event.SerializeAsString();
	RequestTask* task = new RequestTask((int) A1, proto_str);
	queue.push(task);
	return TRUE;
}

PREDICATE(ue_draw_marker, 7)
{ 
	if (clients[(int)A1] == NULL) {
		std::cout << LOG_LABEL << "client " << (int)A1 << " is not connected\n";
		return FALSE;
	}
	sl_pb::KRAmevaEvent ameva_event;
	ameva_event.set_functocall(ameva_event.DrawMarkerAt);
	sl_pb::DrawMarkerAtParams* marker_params = ameva_event.mutable_drawmarkeratparams();
	marker_params->set_id((char*)A2);
	marker_params->set_timestamp((double)A3);
	marker_params->set_marker(get_mesh_type((char*)A4));
	marker_params->set_color((char*)A5);
	marker_params->set_scale((double)A6);
	marker_params->set_unlit((char*)A7);
	std::string proto_str = ameva_event.SerializeAsString();
	RequestTask* task = new RequestTask((int) A1, proto_str);
	queue.push(task);
	return TRUE;
}

PREDICATE(ue_draw_marker, 8)
{ 
	if (clients[(int)A1] == NULL) {
		std::cout << LOG_LABEL << "client " << (int)A1 << " is not connected\n";
		return FALSE;
	}
	sl_pb::KRAmevaEvent ameva_event;
	ameva_event.set_functocall(ameva_event.DrawMarkerTraj);
	sl_pb::DrawMarkerTrajParams* marker_traj_params = ameva_event.mutable_drawmarkertrajparams();
	marker_traj_params->set_id((char*)A2);
	marker_traj_params->set_start((double)A3);
	marker_traj_params->set_end((double)A4);
	marker_traj_params->set_marker(get_mesh_type((char*)A5));
	marker_traj_params->set_color((char*)A6);
	marker_traj_params->set_scale((double)A7);
	marker_traj_params->set_unlit((char*)A7);
	std::string proto_str = ameva_event.SerializeAsString();
	RequestTask* task = new RequestTask((int) A1, proto_str);
	queue.push(task);
	return TRUE;
}

PREDICATE(ue_close_srv, 0)
{ 
	interrupted = 1;
	return TRUE;
}

