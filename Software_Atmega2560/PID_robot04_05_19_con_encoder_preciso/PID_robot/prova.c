double PID_linea_asx_psx(int16_t differenza, uint8_t pend)
{
	if(differenza>=4000 && differenza<5000 && f_possibile_dv==1 && pend==PIANO)
	{
		result_sx=differenza>=4545 || differenza<=4455 ? PID_gira_sx(abs(differenza-4500),differenza>4500?DX:SX,0.7):PID_gira_sx(abs(differenza-4500),differenza>4500?DX:SX,0.5);
	}
	else if((differenza<4000 || differenza>=5000) && pend==PIANO)
	{
		switch(differenza)
		{
			case 1000://acuta dx
			{
				if(f_start_acuta_sx==0)
				{
					f_start_acuta_sx=1;
					set_millis(9);
					result_sx=0.5;
				}
				else if(get_millis(9)<=200)//gira *correttura* a sx
					result_sx=-0.5;
				else
					result_sx=0.5;//*avanti* e *gira* a dx
				break;
			}
			case 1100://acuta sx
			{
				if(f_start_acuta_sx==0)
				{
					f_start_acuta_sx=1;
					set_millis(9);
					result_sx=0.5;
				}
				else if(get_millis(9)>=450)//entro i 450 ms giro *correttura* a dx(150) e vado *avanti*(300)
					result_sx=-0.5;//poi *giro* a sx
				else
					result_sx=0.5;
				break;
			}
			case 2000://stop
			{
				if(f_stop_sx==0)
				{
					f_stop_sx=1;
					set_millis(7);
					result_sx=-0.5;
				}
				else if(get_millis(7)>35)
				result_sx=0;
				else
				result_sx=-0.5;
				break;
			}
			case 2100://indietro
			{
				result_sx=-0.5;
				break;
			}
			case 3000://serve per girare sul proprio perno (usi in gap e verde_dx)
			{
				result_sx=0.5;
				break;
			}
			case 3100://serve per girare sul proprio perno (usi in gap e verde_sx)
			{
				result_sx=-0.5;
				break;
			}
			default://segui linea in condizioni normali
			{
				f_start_acuta_sx=0;
				f_stop_sx=0;
				if(differenza >= 0)
				{
					Yp_asx_psx=k_avanti*(double)differenza+0.5;
				}
				else
				{
					if(differenza>=-100)
					{
						Yp_asx_psx=k1_indietro*(double)differenza+0.5;
					}
					else
					{
						Yp_asx_psx=k2_indietro*(double)(differenza+100);
					}
				}
				result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
				break;
			}
		}
	}
	else if (pend==DISCESA)
	{
		f_start_acuta_sx=0;
		f_stop_sx=0;
		if(differenza >= 0)
		{
			Yp_asx_psx=k_avanti_discesa*(double)differenza+0.4;
		}
		else
		{
			Yp_asx_psx=k1_indietro_discesa*(double)differenza+0.4;
		}
		result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
	}
	else if(pend == SALITA)
	{
		f_start_acuta_sx=0;
		f_stop_sx=0;
		if(!f_bumper)
		{
			if(differenza >= 0)
			{
				Yp_asx_psx=k_avanti*(double)differenza+0.5;
			}
			else
			{
				if(differenza>=-100)
				{
					Yp_asx_psx=k1_indietro*(double)differenza+0.5;
				}
				else
				{
					Yp_asx_psx=k2_indietro*(double)(differenza+100);
				}
			}
		}
		result_sx=Yp_asx_psx>1.7?1.7:Yp_asx_psx<-1.7?-1.7:Yp_asx_psx;
	}
	return result_sx;
}