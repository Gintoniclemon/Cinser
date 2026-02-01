export type Json =
  | string
  | number
  | boolean
  | null
  | { [key: string]: Json | undefined }
  | Json[]

export type Database = {
  // Allows to automatically instantiate createClient with right options
  // instead of createClient<Database, { PostgrestVersion: 'XX' }>(URL, KEY)
  __InternalSupabase: {
    PostgrestVersion: "14.1"
  }
  public: {
    Tables: {
      eurodreams_tirages: {
        Row: {
          annee: number
          created_at: string
          date_tirage: string
          dream_number: number
          id: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_6: number
        }
        Insert: {
          annee: number
          created_at?: string
          date_tirage: string
          dream_number: number
          id?: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_6: number
        }
        Update: {
          annee?: number
          created_at?: string
          date_tirage?: string
          dream_number?: number
          id?: string
          numero_1?: number
          numero_2?: number
          numero_3?: number
          numero_4?: number
          numero_5?: number
          numero_6?: number
        }
        Relationships: []
      }
      euromillions_tirages: {
        Row: {
          annee: number
          created_at: string
          date_tirage: string
          etoile_1: number
          etoile_2: number
          id: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_tirage: number | null
        }
        Insert: {
          annee: number
          created_at?: string
          date_tirage: string
          etoile_1: number
          etoile_2: number
          id?: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_tirage?: number | null
        }
        Update: {
          annee?: number
          created_at?: string
          date_tirage?: string
          etoile_1?: number
          etoile_2?: number
          id?: string
          numero_1?: number
          numero_2?: number
          numero_3?: number
          numero_4?: number
          numero_5?: number
          numero_tirage?: number | null
        }
        Relationships: []
      }
      loto_tirages: {
        Row: {
          annee: number
          created_at: string
          date_tirage: string
          id: string
          jour: number
          mois: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_6: number | null
          numero_chance: number | null
          numero_complementaire: number | null
          type_tirage: string
        }
        Insert: {
          annee: number
          created_at?: string
          date_tirage: string
          id?: string
          jour: number
          mois: string
          numero_1: number
          numero_2: number
          numero_3: number
          numero_4: number
          numero_5: number
          numero_6?: number | null
          numero_chance?: number | null
          numero_complementaire?: number | null
          type_tirage?: string
        }
        Update: {
          annee?: number
          created_at?: string
          date_tirage?: string
          id?: string
          jour?: number
          mois?: string
          numero_1?: number
          numero_2?: number
          numero_3?: number
          numero_4?: number
          numero_5?: number
          numero_6?: number | null
          numero_chance?: number | null
          numero_complementaire?: number | null
          type_tirage?: string
        }
        Relationships: []
      }
      lottery_metadata: {
        Row: {
          game_type: string
          id: string
          last_tirage_date: string | null
          last_update: string
          total_tirages: number
        }
        Insert: {
          game_type: string
          id?: string
          last_tirage_date?: string | null
          last_update?: string
          total_tirages?: number
        }
        Update: {
          game_type?: string
          id?: string
          last_tirage_date?: string | null
          last_update?: string
          total_tirages?: number
        }
        Relationships: []
      }
      lottery_stats: {
        Row: {
          derniere_sortie: number
          ecart_moyen: number
          game_type: string
          id: string
          numero: number
          occurrences: number
          stat_type: string
          temperature: string
          updated_at: string
        }
        Insert: {
          derniere_sortie?: number
          ecart_moyen?: number
          game_type: string
          id?: string
          numero: number
          occurrences?: number
          stat_type: string
          temperature?: string
          updated_at?: string
        }
        Update: {
          derniere_sortie?: number
          ecart_moyen?: number
          game_type?: string
          id?: string
          numero?: number
          occurrences?: number
          stat_type?: string
          temperature?: string
          updated_at?: string
        }
        Relationships: []
      }
    }
    Views: {
      [_ in never]: never
    }
    Functions: {
      [_ in never]: never
    }
    Enums: {
      [_ in never]: never
    }
    CompositeTypes: {
      [_ in never]: never
    }
  }
}

type DatabaseWithoutInternals = Omit<Database, "__InternalSupabase">

type DefaultSchema = DatabaseWithoutInternals[Extract<keyof Database, "public">]

export type Tables<
  DefaultSchemaTableNameOrOptions extends
    | keyof (DefaultSchema["Tables"] & DefaultSchema["Views"])
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals
  }
    ? keyof (DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"] &
        DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Views"])
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals
}
  ? (DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"] &
      DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Views"])[TableName] extends {
      Row: infer R
    }
    ? R
    : never
  : DefaultSchemaTableNameOrOptions extends keyof (DefaultSchema["Tables"] &
        DefaultSchema["Views"])
    ? (DefaultSchema["Tables"] &
        DefaultSchema["Views"])[DefaultSchemaTableNameOrOptions] extends {
        Row: infer R
      }
      ? R
      : never
    : never

export type TablesInsert<
  DefaultSchemaTableNameOrOptions extends
    | keyof DefaultSchema["Tables"]
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals
  }
    ? keyof DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals
}
  ? DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"][TableName] extends {
      Insert: infer I
    }
    ? I
    : never
  : DefaultSchemaTableNameOrOptions extends keyof DefaultSchema["Tables"]
    ? DefaultSchema["Tables"][DefaultSchemaTableNameOrOptions] extends {
        Insert: infer I
      }
      ? I
      : never
    : never

export type TablesUpdate<
  DefaultSchemaTableNameOrOptions extends
    | keyof DefaultSchema["Tables"]
    | { schema: keyof DatabaseWithoutInternals },
  TableName extends DefaultSchemaTableNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals
  }
    ? keyof DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"]
    : never = never,
> = DefaultSchemaTableNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals
}
  ? DatabaseWithoutInternals[DefaultSchemaTableNameOrOptions["schema"]]["Tables"][TableName] extends {
      Update: infer U
    }
    ? U
    : never
  : DefaultSchemaTableNameOrOptions extends keyof DefaultSchema["Tables"]
    ? DefaultSchema["Tables"][DefaultSchemaTableNameOrOptions] extends {
        Update: infer U
      }
      ? U
      : never
    : never

export type Enums<
  DefaultSchemaEnumNameOrOptions extends
    | keyof DefaultSchema["Enums"]
    | { schema: keyof DatabaseWithoutInternals },
  EnumName extends DefaultSchemaEnumNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals
  }
    ? keyof DatabaseWithoutInternals[DefaultSchemaEnumNameOrOptions["schema"]]["Enums"]
    : never = never,
> = DefaultSchemaEnumNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals
}
  ? DatabaseWithoutInternals[DefaultSchemaEnumNameOrOptions["schema"]]["Enums"][EnumName]
  : DefaultSchemaEnumNameOrOptions extends keyof DefaultSchema["Enums"]
    ? DefaultSchema["Enums"][DefaultSchemaEnumNameOrOptions]
    : never

export type CompositeTypes<
  PublicCompositeTypeNameOrOptions extends
    | keyof DefaultSchema["CompositeTypes"]
    | { schema: keyof DatabaseWithoutInternals },
  CompositeTypeName extends PublicCompositeTypeNameOrOptions extends {
    schema: keyof DatabaseWithoutInternals
  }
    ? keyof DatabaseWithoutInternals[PublicCompositeTypeNameOrOptions["schema"]]["CompositeTypes"]
    : never = never,
> = PublicCompositeTypeNameOrOptions extends {
  schema: keyof DatabaseWithoutInternals
}
  ? DatabaseWithoutInternals[PublicCompositeTypeNameOrOptions["schema"]]["CompositeTypes"][CompositeTypeName]
  : PublicCompositeTypeNameOrOptions extends keyof DefaultSchema["CompositeTypes"]
    ? DefaultSchema["CompositeTypes"][PublicCompositeTypeNameOrOptions]
    : never

export const Constants = {
  public: {
    Enums: {},
  },
} as const
