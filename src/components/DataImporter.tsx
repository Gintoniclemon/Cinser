import React, { useState } from "react";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { useToast } from "@/hooks/use-toast";
import { supabase } from "@/integrations/supabase/client";
import { Loader2, Upload, RefreshCw, FileSpreadsheet, Zap } from "lucide-react";
import type { GameType } from "@/pages/Index";

interface DataImporterProps {
  selectedGame: GameType;
  onDataImported: () => void;
}

export const DataImporter = ({ selectedGame, onDataImported }: DataImporterProps) => {
  const [isLoadingApi, setIsLoadingApi] = useState(false);
  const [isLoadingFile, setIsLoadingFile] = useState(false);
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const { toast } = useToast();

  const handleApiSync = async () => {
    setIsLoadingApi(true);
    try {
      const { data, error } = await supabase.functions.invoke("lottery-sync", {
        body: { action: "sync_fdj_api", game: selectedGame },
      });

      if (error) throw error;

      toast({
        title: "Synchronisation réussie",
        description: `${data.inserted || 0} nouveaux tirages importés via l'API FDJ.`,
      });
      onDataImported();
    } catch (error: any) {
      toast({
        title: "Erreur de synchronisation",
        description: error.message || "Impossible de synchroniser avec l'API FDJ.",
        variant: "destructive",
      });
    } finally {
      setIsLoadingApi(false);
    }
  };

  const handleFileUpload = async () => {
    if (!selectedFile) {
      toast({
        title: "Aucun fichier",
        description: "Veuillez sélectionner un fichier Excel.",
        variant: "destructive",
      });
      return;
    }

    setIsLoadingFile(true);
    try {
      const base64 = await fileToBase64(selectedFile);
      
      const { data, error } = await supabase.functions.invoke("lottery-sync", {
        body: { 
          action: "import_excel",
          game: selectedGame,
          file: base64,
          filename: selectedFile.name
        },
      });

      if (error) throw error;

      toast({
        title: "Import réussi",
        description: `${data.inserted || 0} tirages importés depuis le fichier.`,
      });
      setSelectedFile(null);
      onDataImported();
    } catch (error: any) {
      toast({
        title: "Erreur d'import",
        description: error.message || "Impossible d'importer le fichier.",
        variant: "destructive",
      });
    } finally {
      setIsLoadingFile(false);
    }
  };

  const getGameName = () => {
    switch (selectedGame) {
      case "loto": return "Loto";
      case "euromillions": return "EuroMillions";
      case "eurodreams": return "EuroDreams";
      case "crescendo": return "Crescendo";
    }
  };

  return (
    <div className="grid gap-6 md:grid-cols-2">
      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Zap className="h-5 w-5 text-euromillions" />
            MAJ Automatique API FDJ
          </CardTitle>
          <CardDescription>
            Synchronise les derniers tirages {getGameName()} depuis l'API officielle FDJ
          </CardDescription>
        </CardHeader>
        <CardContent>
          <Button
            onClick={handleApiSync}
            disabled={isLoadingApi}
            className="w-full"
            size="lg"
          >
            {isLoadingApi ? (
              <>
                <Loader2 className="mr-2 h-4 w-4 animate-spin" />
                Synchronisation...
              </>
            ) : (
              <>
                <RefreshCw className="mr-2 h-4 w-4" />
                Synchroniser {getGameName()}
              </>
            )}
          </Button>
        </CardContent>
      </Card>

      <Card>
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <FileSpreadsheet className="h-5 w-5 text-loto" />
            Import Manuel Excel
          </CardTitle>
          <CardDescription>
            Importez un fichier Excel avec l'historique des tirages
          </CardDescription>
        </CardHeader>
        <CardContent className="space-y-4">
          <div>
            <Label htmlFor="file">Fichier Excel (.xlsx, .xls)</Label>
            <Input
              id="file"
              type="file"
              accept=".xlsx,.xls"
              onChange={(e: React.ChangeEvent<HTMLInputElement>) => setSelectedFile(e.target.files?.[0] || null)}
              className="mt-2"
            />
          </div>
          <Button
            onClick={handleFileUpload}
            disabled={isLoadingFile || !selectedFile}
            className="w-full"
            variant="secondary"
          >
            {isLoadingFile ? (
              <>
                <Loader2 className="mr-2 h-4 w-4 animate-spin" />
                Import en cours...
              </>
            ) : (
              <>
                <Upload className="mr-2 h-4 w-4" />
                Importer le fichier
              </>
            )}
          </Button>
        </CardContent>
      </Card>
    </div>
  );
};

function fileToBase64(file: File): Promise<string> {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.readAsDataURL(file);
    reader.onload = () => {
      const result = reader.result as string;
      const base64 = result.split(",")[1];
      resolve(base64);
    };
    reader.onerror = reject;
  });
}
